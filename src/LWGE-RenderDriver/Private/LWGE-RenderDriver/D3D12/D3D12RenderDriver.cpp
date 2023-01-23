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
    constexpr static uint32_t MAX_CBV_SRV_UAV_DESCRIPTORS = 1'000'000;
    constexpr static uint32_t MAX_SAMPLER_DESCRIPTORS = 1024;

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

        D3D12_ROOT_PARAMETER1 rootsig_constants = {
            .ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
            .Constants = {
                .ShaderRegister = 0,
                .RegisterSpace = 0,
                .Num32BitValues = 4
            },
            .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
        };
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootsig_desc = {
            .Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
            .Desc_1_1 = {
                .NumParameters = 1,
                .pParameters = &rootsig_constants,
                .NumStaticSamplers = 0,
                .Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
                       | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
            }
        };
        ComPtr<ID3DBlob> rootsig_blob = {};
        ComPtr<ID3DBlob> rootsig_error = {};
        D3D12SerializeVersionedRootSignature(&rootsig_desc, &rootsig_blob, &rootsig_error);
        m_device->CreateRootSignature(0, rootsig_blob->GetBufferPointer(),
            rootsig_blob->GetBufferSize(), IID_PPV_ARGS(&m_rootsig));

        D3D12_DESCRIPTOR_HEAP_DESC shader_descriptor_heap_desc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = MAX_CBV_SRV_UAV_DESCRIPTORS,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            .NodeMask = 0
        };
        m_device->CreateDescriptorHeap(&shader_descriptor_heap_desc,
            IID_PPV_ARGS(&m_cbv_srv_uav_descriptor_heap));
        shader_descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        shader_descriptor_heap_desc.NumDescriptors = MAX_SAMPLER_DESCRIPTORS;
        m_device->CreateDescriptorHeap(&shader_descriptor_heap_desc,
            IID_PPV_ARGS(&m_sampler_descriptor_heap));

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

        ComPtr<ID3D12Resource2> resource = nullptr;
        D3D12_HEAP_PROPERTIES heap_desc = {
            .Type = D3D12_HEAP_TYPE_DEFAULT,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
        };
        D3D12_RESOURCE_DESC1 desca = {
            .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .Alignment = 0,
            .Width = 16384,
            .Height = 16384,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_R16G16B16A16_FLOAT,
            .SampleDesc = { 1, 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = D3D12_RESOURCE_FLAG_NONE,
            .SamplerFeedbackMipRegion = {}
        };
        m_device->CreateCommittedResource3(&heap_desc, D3D12_HEAP_FLAG_NONE,
            &desca, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr,
            0, nullptr, IID_PPV_ARGS(&resource));

    }

    D3D12RenderDriver::~D3D12RenderDriver()
    {
        gpu_wait_idle();
        empty_deletion_queues(~0ull);
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
        empty_deletion_queues(frame_number);
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

    [[nodiscard]] D3D12_HEAP_TYPE translate_heap_type(ResourceHeap heap) noexcept
    {
        switch (heap)
        {
        case lwge::rd::ResourceHeap::Vidmem:
            return D3D12_HEAP_TYPE_DEFAULT;
        case lwge::rd::ResourceHeap::CPU:
            return D3D12_HEAP_TYPE_UPLOAD;
        case lwge::rd::ResourceHeap::Readback:
            return D3D12_HEAP_TYPE_READBACK;
        default:
            std::unreachable();
        }
    }

    BufferHandle D3D12RenderDriver::create_buffer(const BufferDesc& desc) noexcept
    {
        auto handle = m_buffer_pool.insert(0);
        auto& buffer = m_buffer_pool[handle];
        D3D12_HEAP_PROPERTIES heap_props = {
            .Type = translate_heap_type(desc.heap),
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 0,
            .VisibleNodeMask = 0
        };
        D3D12_RESOURCE_DESC1 resource_desc = {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = desc.size,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_UNKNOWN,
            .SampleDesc = { .Count = 1, .Quality = 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAG_NONE,
            .SamplerFeedbackMipRegion = {}
        };
        m_device->CreateCommittedResource3(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc,
            D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&buffer.resource));
        return handle;
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

    void D3D12RenderDriver::destroy_buffer(BufferHandle buffer) noexcept
    {
        std::lock_guard<std::mutex> lock(m_buffer_deletion_queue.mutex);
        m_buffer_deletion_queue.queue.push_back({ buffer,
            m_frame_counter.load(std::memory_order_relaxed) + MAX_CONCURRENT_GPU_FRAMES - 1 });
    }

    void D3D12RenderDriver::destroy_image(ImageHandle image) noexcept
    {
        std::lock_guard<std::mutex> lock(m_image_deletion_queue.mutex);
        m_image_deletion_queue.queue.push_back({ image,
            m_frame_counter.load(std::memory_order_relaxed) + MAX_CONCURRENT_GPU_FRAMES - 1});
    }

    void D3D12RenderDriver::destroy_pipeline(PipelineHandle pipe) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pipeline_deletion_queue.mutex);
        m_pipeline_deletion_queue.queue.push_back({ pipe,
            m_frame_counter.load(std::memory_order_relaxed) + MAX_CONCURRENT_GPU_FRAMES - 1});
    }

    void D3D12RenderDriver::destroy_resource_deferred(ComPtr<IDXGISwapChain4> swapchain) noexcept
    {
        std::lock_guard<std::mutex> lock(m_swapchain_deletion_queue.mutex);
        m_swapchain_deletion_queue.queue.push_back({ swapchain,
            m_frame_counter.load(std::memory_order_relaxed) + MAX_CONCURRENT_GPU_FRAMES - 1 });
    }

    void D3D12RenderDriver::empty_deletion_queues(uint64_t frame) noexcept
    {
        auto create_range = [frame](auto& vec) noexcept {
            return std::ranges::remove_if(vec, [frame](auto& e) noexcept {
                    if (frame > e.frame)
                    {
                        return true;
                    }
                    return false;
                });
        };

        auto fill_resource_vector = [](auto& from, auto& to, auto& pool) mutable noexcept {
            to.reserve(from.size());
            for (auto& f : from)
            {
                pool[f.element].resource->Release();
                to.push_back(f.element);
            }
        };

        using Lock = std::lock_guard<std::mutex>;

        auto delete_resources = [create_range, fill_resource_vector](
            auto& pool, auto& deletion_queue) mutable {
            /// Even though in most cases those locks here are a double-lock
            /// With remove_bulk, it allows an async-worker to create resources
            /// whilst the deletion queues are getting emptied.
            Lock lock(deletion_queue.mutex);
            auto range = create_range(deletion_queue.queue);
            std::vector<decltype(decltype(deletion_queue.queue)::value_type::element)> handles_to_destroy;
            fill_resource_vector(range, handles_to_destroy, pool);
            pool.remove_bulk(handles_to_destroy);
            deletion_queue.queue.erase(range.begin(), range.end());
        };
        delete_resources(m_buffer_pool, m_buffer_deletion_queue);
        delete_resources(m_image_pool, m_image_deletion_queue);
        delete_resources(m_pipeline_pool, m_pipeline_deletion_queue);

        Lock lock(m_swapchain_deletion_queue.mutex);
        auto range = create_range(m_swapchain_deletion_queue.queue);
        m_swapchain_deletion_queue.queue.erase(range.begin(), range.end());
    }
}
#endif // LWGE_BUILD_D3D12
