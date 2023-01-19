#pragma once
#if LWGE_BUILD_D3D12

#include "LWGE-RenderDriver/RenderDriver.hpp"
#include "LWGE-RenderDriver/FrameContext.hpp"
#include "LWGE-RenderDriver/HandleResourcePool.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Includes.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Swapchain.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12CommandListRecycler.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12CommandList.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <mutex>
#include <vector>

namespace lwge::rd::d3d12
{
    struct D3D12Buffer
    {
        uint32_t srv_idx;
        uint32_t uav_idx;
        OwningPtr<ID3D12Resource2> resource;
    };

    struct D3D12Image
    {
        uint32_t srv_idx;
        uint32_t uav_idx;
        OwningPtr<ID3D12Resource2> resource;
    };

    struct D3D12Pipeline
    {

    };

    struct D3D12FrameContext : public FrameContext
    {
#pragma warning(push)
#pragma warning(disable : 4324)
        struct alignas(std::hardware_destructive_interference_size) ThreadData
        {
            D3D12CommandListRecycler direct_queue_recycler;
            D3D12CommandListRecycler compute_queue_recycler;
            std::vector<D3D12CommandList> command_lists;
        };
#pragma warning(pop)
        std::vector<ThreadData> thread_data;
        ComPtr<ID3D12Fence1> direct_fence;
        ComPtr<ID3D12Fence1> compute_fence;
        uint64_t fence_val;
    };

    class D3D12RenderDriver final : public RenderDriver
    {
    public:
        D3D12RenderDriver(const RenderDriverDesc& desc);
        virtual ~D3D12RenderDriver() override;

        virtual [[nodiscard]] NonOwningPtr<FrameContext> start_frame() noexcept override;
        virtual void end_frame(NonOwningPtr<FrameContext> frame) noexcept override;
        virtual void gpu_wait_idle() override;

        virtual [[nodiscard]] NonOwningPtr<CopyCommandList> get_async_copy_cmdlist(
            uint32_t thread_idx) noexcept override;
        virtual [[nodiscard]] NonOwningPtr<ComputeCommandList> get_compute_cmdlist(
            NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept override;
        virtual [[nodiscard]] NonOwningPtr<GraphicsCommandList> get_graphics_cmdlist(
            NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept override;

        virtual [[nodiscard]] BufferHandle create_buffer(const BufferDesc& desc) noexcept override;
        virtual [[nodiscard]] ImageHandle create_image(const ImageDesc& desc) noexcept override;
        virtual [[nodiscard]] PipelineHandle create_pipeline(const GraphicsPipelineDesc& desc) noexcept override;
        virtual [[nodiscard]] PipelineHandle create_pipeline(const ComputePipelineDesc& desc) noexcept override;

        [[nodiscard]] NonOwningPtr<ID3D12Device10> get_d3d12device() const noexcept
        { return m_device.Get(); }
        [[nodiscard]] NonOwningPtr<IDXGIFactory7> get_dxgi_factory() const noexcept
        { return m_factory.Get(); }
        [[nodiscard]] NonOwningPtr<ID3D12CommandQueue> get_direct_queue() const noexcept
        { return m_direct_queue.Get(); }

        void destroy_resource_deferred(ComPtr<IDXGISwapChain4> swapchain) noexcept;

    private:
        void empty_deletion_queues() noexcept;

    private:
        ComPtr<IDXGIFactory7> m_factory;
        ComPtr<IDXGIAdapter4> m_adapter;
        ComPtr<ID3D12Device10> m_device;
        ComPtr<ID3D12CommandQueue> m_direct_queue;
        ComPtr<ID3D12CommandQueue> m_compute_queue;
        ComPtr<ID3D12CommandQueue> m_copy_queue;
        ComPtr<ID3D12RootSignature> m_rootsig;
        ComPtr<ID3D12DescriptorHeap> m_cbv_srv_uav_descriptor_heap;
        ComPtr<ID3D12DescriptorHeap> m_sampler_descriptor_heap;

        std::atomic<uint64_t> m_frame_counter;

        template<typename T>
        struct DeletionQueue
        {
            std::mutex mutex;
            struct Resource
            {
                T element;
                uint64_t frame;
            };
            std::vector<Resource> queue;
        };
        DeletionQueue<ComPtr<IDXGISwapChain4>> m_swapchain_deletion_queue;

        std::array<D3D12FrameContext, MAX_CONCURRENT_GPU_FRAMES> m_frames;

        HandleResourcePool<Buffer, D3D12Buffer, 0x80000, true> m_buffer_pool;
        HandleResourcePool<Image, D3D12Image, 0x80000, true> m_image_pool;
        HandleResourcePool<Pipeline, D3D12Pipeline, 0x40000, true> m_pipeline_pool;
    };
}
#endif // LWGE_BUILD_D3D12
