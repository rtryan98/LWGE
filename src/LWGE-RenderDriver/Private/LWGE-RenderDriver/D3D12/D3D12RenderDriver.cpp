#if LWGE_BUILD_D3D12
#include "LWGE-RenderDriver/D3D12/D3D12RenderDriver.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Util.hpp"
#include "LWGE-RenderDriver/Util.hpp"

#include <cmath>
#include <ranges>

extern "C"
{
    __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 608;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

namespace lwge::rd::d3d12
{
    DWORD wait_for_fence(ID3D12Fence1* fence, uint64_t target_value, uint32_t timeout)
    {
        if (fence->GetCompletedValue() < target_value)
        {
            HANDLE event = CreateEvent(NULL, FALSE, FALSE, NULL);
            throw_if_failed(fence->SetEventOnCompletion(target_value, event));
            if (!event)
            {
                throw std::exception();
            }
            return WaitForSingleObject(event, timeout);
        }
        return WAIT_FAILED;
    }

    ComPtr<IDXGIFactory7> create_dxgi_factory()
    {
        uint32_t factory_flags = 0u;
#if LWGE_GRAPHICS_DEBUG
        factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
        ComPtr<ID3D12Debug> debug = nullptr;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
        {
            debug->EnableDebugLayer();
        }
#endif
        ComPtr<IDXGIFactory7> result = nullptr;
        throw_if_failed(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&result)));
        return result;
    }

    ComPtr<IDXGIAdapter4> create_adapter(IDXGIFactory7* factory)
    {
        ComPtr<IDXGIAdapter4> result = nullptr;
        throw_if_failed(factory->EnumAdapterByGpuPreference(0,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&result)));
        return result;
    }

    ComPtr<ID3D12Device10> create_device(IDXGIAdapter4* adapter, D3D_FEATURE_LEVEL feature_level)
    {
        ComPtr<ID3D12Device10> result = nullptr;
        throw_if_failed(D3D12CreateDevice(adapter, feature_level, IID_PPV_ARGS(&result)));
        return result;
    }

    ComPtr<ID3D12CommandQueue> create_command_queue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {
            .Type = type,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0
        };
        ComPtr<ID3D12CommandQueue> result = nullptr;
        throw_if_failed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&result)));
        return result;
    }

    D3D12RenderDriver::D3D12RenderDriver(const RenderDriverDesc& desc)
        : RenderDriver(desc),
        m_factory(create_dxgi_factory()),
        m_adapter(create_adapter(m_factory.Get())),
        m_device(create_device(m_adapter.Get(), D3D_FEATURE_LEVEL_12_2)),
        m_direct_queue(create_command_queue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT)),
        m_compute_queue(create_command_queue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE)),
        m_copy_queue(create_command_queue(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COPY))
    {
        DXGI_ADAPTER_DESC adapter_desc = {};
        m_adapter->GetDesc(&adapter_desc);
        m_vendor = get_vendor_from_pci_id(adapter_desc.VendorId);
        for (uint32_t frame = 0; frame < MAX_CONCURRENT_GPU_FRAMES; frame++)
        {
            auto& fc = m_frames[frame];
            fc.fence_val = 0;
            fc.thread_data.reserve(std::max(m_thread_count, 1u));
            m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fc.direct_fence));
            m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fc.compute_fence));
            for (uint32_t thread = 0; thread < std::max(m_thread_count, 1u); thread++)
            {
                fc.thread_data.push_back({
                    .direct_queue_recycler = D3D12CommandListRecycler(m_device.Get(),
                        D3D12_COMMAND_LIST_TYPE_DIRECT),
                    .compute_queue_recycler = D3D12CommandListRecycler(m_device.Get(),
                        D3D12_COMMAND_LIST_TYPE_COMPUTE)
                    });
            }
        }
    }

    D3D12RenderDriver::~D3D12RenderDriver()
    {
        gpu_wait_idle();
    }

    void wait_on_frame_context(D3D12FrameContext* frame)
    {
        wait_for_fence(frame->direct_fence.Get(), frame->fence_val, INFINITE);
        wait_for_fence(frame->compute_fence.Get(), frame->fence_val, INFINITE);
    }

    NonOwningPtr<FrameContext> D3D12RenderDriver::start_frame() noexcept
    {
        auto frame_number = m_frame_counter.fetch_add(1);
        auto frame = &m_frames[frame_number % MAX_CONCURRENT_GPU_FRAMES];
        wait_on_frame_context(frame);
        for (auto& td : frame->thread_data)
        {
            td.direct_queue_recycler.reset();
            td.compute_queue_recycler.reset();
        }
        empty_deletion_queues();
        return frame;
    }

    void D3D12RenderDriver::end_frame(NonOwningPtr<FrameContext> frame) noexcept
    {
        auto fc = static_cast<D3D12FrameContext*>(frame);
        auto val = ++fc->fence_val;
        m_direct_queue->Signal(fc->direct_fence.Get(), val);
        m_compute_queue->Signal(fc->compute_fence.Get(), val);
    }

    void D3D12RenderDriver::gpu_wait_idle()
    {
        ComPtr<ID3D12Fence1> fence = nullptr;
        m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        uint64_t target_value = 1;
        m_direct_queue->Signal(fence.Get(), target_value);
        wait_for_fence(fence.Get(), target_value++, INFINITE);
        m_compute_queue->Signal(fence.Get(), target_value);
        wait_for_fence(fence.Get(), target_value++, INFINITE);
        m_copy_queue->Signal(fence.Get(), target_value);
        wait_for_fence(fence.Get(), target_value, INFINITE);
    }

    NonOwningPtr<CopyCommandList> D3D12RenderDriver::get_async_copy_cmdlist(uint32_t thread_idx) noexcept
    {
        thread_idx;
        return NonOwningPtr<CopyCommandList>();
    }

    NonOwningPtr<ComputeCommandList> D3D12RenderDriver::get_compute_cmdlist(NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept
    {
        auto& fctd = static_cast<D3D12FrameContext*>(frame)->thread_data[thread_idx];
        auto& alloc = fctd.compute_queue_recycler;
        auto cmd = alloc.get_or_create_cmd_list();
        auto& result = fctd.command_lists.emplace_back(D3D12CommandList(this, alloc.get_allocator(), cmd));
        return NonOwningPtr<GraphicsCommandList>(&result);
    }

    NonOwningPtr<GraphicsCommandList> D3D12RenderDriver::get_graphics_cmdlist(NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept
    {
        auto& fctd = static_cast<D3D12FrameContext*>(frame)->thread_data[thread_idx];
        auto& alloc = fctd.direct_queue_recycler;
        auto cmd = alloc.get_or_create_cmd_list();
        auto& result = fctd.command_lists.emplace_back(D3D12CommandList(this, alloc.get_allocator(), cmd));
        return NonOwningPtr<GraphicsCommandList>(&result);
    }

    BufferHandle D3D12RenderDriver::create_buffer(const BufferDesc& desc) noexcept
    {
        desc;
        return BufferHandle();
    }

    ImageHandle D3D12RenderDriver::create_image(const ImageDesc& desc) noexcept
    {
        desc;
        return ImageHandle();
    }

    PipelineHandle D3D12RenderDriver::create_pipeline(const GraphicsPipelineDesc& desc) noexcept
    {
        desc;
        return PipelineHandle();
    }

    PipelineHandle D3D12RenderDriver::create_pipeline(const ComputePipelineDesc& desc) noexcept
    {
        desc;
        return PipelineHandle();
    }

    void D3D12RenderDriver::destroy_resource_deferred(ComPtr<IDXGISwapChain4> swapchain) noexcept
    {
        std::lock_guard<std::mutex> lock(m_swapchain_deletion_queue.mutex);
        m_swapchain_deletion_queue.queue.push_back({ swapchain, m_frame_counter.load(std::memory_order_relaxed) });
    }

    void D3D12RenderDriver::empty_deletion_queues() noexcept
    {
        uint64_t current_frame = m_frame_counter.load();
        {
            std::lock_guard<std::mutex> lock(m_swapchain_deletion_queue.mutex);
            auto range = std::ranges::remove_if(m_swapchain_deletion_queue.queue,
                [current_frame](auto& e) noexcept -> bool {
                    if (current_frame > e.frame)
                    {
                        e.element->Release();
                        return true;
                    }
                    return false;
                });
            m_swapchain_deletion_queue.queue.erase(range.begin(), range.end());
        }
    }
}
#endif // LWGE_BUILD_D3D12
