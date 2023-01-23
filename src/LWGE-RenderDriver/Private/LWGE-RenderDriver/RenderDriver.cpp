#include "LWGE-RenderDriver/RenderDriver.hpp"
#include "LWGE-RenderDriver/CommandList.hpp"
#include "LWGE-RenderDriver/HandleResourcePool.hpp"
#include "LWGE-RenderDriver/Util.hpp"
#include "LWGE-RenderDriver/CommandListRecycler.hpp"
#include "LWGE-RenderDriver/Constants.hpp"

#include <algorithm>
#include <utility>
#include <ranges>
#include "LWGE-RenderDriver/D3D12Includes.hpp"

extern "C"
{
    __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 608;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

namespace lwge::rd
{
    constexpr static uint32_t MAX_CBV_SRV_UAV_DESCRIPTORS = 1'000'000;
    constexpr static uint32_t MAX_SAMPLER_DESCRIPTORS = 1024;

    struct RenderDriver::ResourcePools
    {
        HandleResourcePool<Buffer, 0x80000, true> m_buffer_pool;
        HandleResourcePool<Image, 0x80000, true> m_image_pool;
        HandleResourcePool<Pipeline, 0x40000, true> m_pipeline_pool;
    };

    struct FrameContext
    {
#pragma warning(push)
#pragma warning(disable : 4324)
        struct alignas(std::hardware_destructive_interference_size) ThreadData
        {
            CommandListRecycler direct_queue_recycler;
            CommandListRecycler compute_queue_recycler;
            std::vector<GraphicsCommandList> graphics_command_lists;
            std::vector<ComputeCommandList> compute_command_lists;
        };
#pragma warning(pop)
        std::vector<ThreadData> thread_data;
        ComPtr<ID3D12Fence1> direct_fence;
        ComPtr<ID3D12Fence1> compute_fence;
        uint64_t fence_val;
    };

    void wait_on_frame_context(FrameContext* frame)
    {
        wait_for_fence(frame->direct_fence.Get(), frame->fence_val, INFINITE);
        wait_for_fence(frame->compute_fence.Get(), frame->fence_val, INFINITE);
    }

    IDXGIFactory7* create_dxgi_factory()
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
        IDXGIFactory7* result = nullptr;
        throw_if_failed(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&result)));
        return result;
    }

    IDXGIAdapter4* create_adapter(IDXGIFactory7* factory)
    {
        IDXGIAdapter4* result = nullptr;
        throw_if_failed(factory->EnumAdapterByGpuPreference(0,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&result)));
        return result;
    }

    ID3D12Device10* create_device(IDXGIAdapter4* adapter, D3D_FEATURE_LEVEL feature_level)
    {
        ID3D12Device10* result = nullptr;
        throw_if_failed(D3D12CreateDevice(adapter, feature_level, IID_PPV_ARGS(&result)));
        return result;
    }

    ID3D12CommandQueue* create_command_queue(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
    {
        D3D12_COMMAND_QUEUE_DESC desc = {
            .Type = type,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0
        };
        ID3D12CommandQueue* result = nullptr;
        throw_if_failed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&result)));
        return result;
    }

    RenderDriver::~RenderDriver()
    {
        gpu_wait_idle();
        empty_deletion_queues(~0ull);
        delete m_pools;
        m_sampler_descriptor_heap->Release();
        m_cbv_srv_uav_descriptor_heap->Release();
        m_rootsig->Release();
        m_copy_queue->Release();
        m_compute_queue->Release();
        m_direct_queue->Release();
        m_device->Release();
        m_adapter->Release();
        m_factory->Release();
    }

    RenderDriver::RenderDriver(const RenderDriverDesc& desc)
        : m_vendor(Vendor::Unknown), m_thread_count(desc.thread_count),
        m_factory(create_dxgi_factory()),
        m_adapter(create_adapter(m_factory)),
        m_device(create_device(m_adapter, D3D_FEATURE_LEVEL_12_2)),
        m_direct_queue(create_command_queue(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT)),
        m_compute_queue(create_command_queue(m_device, D3D12_COMMAND_LIST_TYPE_COMPUTE)),
        m_copy_queue(create_command_queue(m_device, D3D12_COMMAND_LIST_TYPE_COPY)),
        m_pools(new RenderDriver::ResourcePools{})
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

        m_frames.resize(MAX_CONCURRENT_GPU_FRAMES);
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
                    .direct_queue_recycler = CommandListRecycler(m_device,
                        D3D12_COMMAND_LIST_TYPE_DIRECT),
                    .compute_queue_recycler = CommandListRecycler(m_device,
                        D3D12_COMMAND_LIST_TYPE_COMPUTE)
                    });
            }
        }
    }

    NonOwningPtr<FrameContext> RenderDriver::start_frame() noexcept
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

    void RenderDriver::end_frame(NonOwningPtr<FrameContext> frame) noexcept
    {
        auto val = ++frame->fence_val;
        m_direct_queue->Signal(frame->direct_fence.Get(), val);
        m_compute_queue->Signal(frame->compute_fence.Get(), val);
    }

    void RenderDriver::gpu_wait_idle()
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

    NonOwningPtr<ComputeCommandList> RenderDriver::get_compute_cmdlist(NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept
    {
        auto& fctd = frame->thread_data[thread_idx];
        auto& alloc = fctd.compute_queue_recycler;
        auto cmd = alloc.get_or_create_cmd_list();
        auto& result = fctd.compute_command_lists.emplace_back(ComputeCommandList(*alloc.get_allocator(), cmd));
        return NonOwningPtr<GraphicsCommandList>(&result);
    }

    NonOwningPtr<GraphicsCommandList> RenderDriver::get_graphics_cmdlist(NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept
    {
        auto& fctd = frame->thread_data[thread_idx];
        auto& alloc = fctd.direct_queue_recycler;
        auto cmd = alloc.get_or_create_cmd_list();
        auto& result = fctd.graphics_command_lists.emplace_back(GraphicsCommandList(*alloc.get_allocator(), cmd));
        return NonOwningPtr<GraphicsCommandList>(&result);
    }

    [[nodiscard]] D3D12_HEAP_TYPE translate_heap_type(ResourceHeap heap) noexcept
    {
        switch (heap)
        {
        case ResourceHeap::Vidmem:
            return D3D12_HEAP_TYPE_DEFAULT;
        case ResourceHeap::CPU:
            return D3D12_HEAP_TYPE_UPLOAD;
        case ResourceHeap::Readback:
            return D3D12_HEAP_TYPE_READBACK;
        default:
            std::unreachable();
        }
    }

    BufferHandle RenderDriver::create_buffer(const BufferDesc& desc) noexcept
    {
        auto handle = m_pools->m_buffer_pool.insert(0);
        auto& buffer = m_pools->m_buffer_pool[handle];
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
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAG_NONE,
            .SamplerFeedbackMipRegion = {}
        };
        m_device->CreateCommittedResource3(&heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc,
            D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&buffer.resource));
        return handle;
    }

    ImageHandle RenderDriver::create_image(const ImageDesc& desc) noexcept
    {
        desc;
        return ImageHandle();
    }

    PipelineHandle RenderDriver::create_pipeline(const GraphicsPipelineDesc& desc) noexcept
    {
        desc;
        return PipelineHandle();
    }

    PipelineHandle RenderDriver::create_pipeline(const ComputePipelineDesc& desc) noexcept
    {
        desc;
        return PipelineHandle();
    }

    void RenderDriver::destroy_buffer(BufferHandle buffer) noexcept
    {
        std::lock_guard<std::mutex> lock(m_buffer_deletion_queue.mutex);
        m_buffer_deletion_queue.queue.push_back({ buffer,
            m_frame_counter.load(std::memory_order_relaxed) + MAX_CONCURRENT_GPU_FRAMES - 1 });
    }

    void RenderDriver::destroy_image(ImageHandle image) noexcept
    {
        std::lock_guard<std::mutex> lock(m_image_deletion_queue.mutex);
        m_image_deletion_queue.queue.push_back({ image,
            m_frame_counter.load(std::memory_order_relaxed) + MAX_CONCURRENT_GPU_FRAMES - 1 });
    }

    void RenderDriver::destroy_pipeline(PipelineHandle pipe) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pipeline_deletion_queue.mutex);
        m_pipeline_deletion_queue.queue.push_back({ pipe,
            m_frame_counter.load(std::memory_order_relaxed) + MAX_CONCURRENT_GPU_FRAMES - 1 });
    }

    void RenderDriver::destroy_resource_deferred(IDXGISwapChain4* swapchain) noexcept
    {
        std::lock_guard<std::mutex> lock(m_swapchain_deletion_queue.mutex);
        m_swapchain_deletion_queue.queue.push_back({ swapchain,
            m_frame_counter.load(std::memory_order_relaxed) + MAX_CONCURRENT_GPU_FRAMES - 1 });
    }

    void RenderDriver::empty_deletion_queues(uint64_t frame) noexcept
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
        delete_resources(m_pools->m_buffer_pool, m_buffer_deletion_queue);
        delete_resources(m_pools->m_image_pool, m_image_deletion_queue);
        delete_resources(m_pools->m_pipeline_pool, m_pipeline_deletion_queue);

        Lock lock(m_swapchain_deletion_queue.mutex);
        auto range = create_range(m_swapchain_deletion_queue.queue);
        for (auto& e : range)
        {
            e.element->Release();
        }
        m_swapchain_deletion_queue.queue.erase(range.begin(), range.end());
    }
}
