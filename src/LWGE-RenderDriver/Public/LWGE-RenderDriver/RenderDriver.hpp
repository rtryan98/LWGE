#pragma once

#include "LWGE-RenderDriver/Resource.hpp"
#include "LWGE-RenderDriver/Swapchain.hpp"
#include <LWGE-Common/Pointer.hpp>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

struct IDXGIFactory7;
struct IDXGIAdapter4;
struct ID3D12Device10;
struct ID3D12CommandQueue;
struct ID3D12RootSignature;
struct ID3D12DescriptorHeap;
struct ID3D12CommandSignature;
struct IDXGISwapChain4;

namespace lwge::rd
{
    struct FrameContext;
    class CopyCommandList;
    class ComputeCommandList;
    class GraphicsCommandList;

    enum class Vendor : uint32_t
    {
        AMD,
        INTEL,
        NVIDIA,
        Unknown = ~0u,
    };

    struct RenderDriverDesc
    {
        uint32_t thread_count;
    };

    struct SwapchainDestroyPayload
    {
        OwningPtr<IDXGISwapChain4> swapchain;
        OwningPtr<ID3D12DescriptorHeap> descriptor_heap;
    };

    struct Indirect
    {
        OwningPtr<ID3D12CommandSignature> dispatch_indirect;
        OwningPtr<ID3D12CommandSignature> dispatch_rays_indirect;
        OwningPtr<ID3D12CommandSignature> draw_indirect;
        OwningPtr<ID3D12CommandSignature> draw_indexed_indirect;
        OwningPtr<ID3D12CommandSignature> dispatch_mesh_indirect;
    };

    class RenderDriver
    {
    public:
        RenderDriver(const RenderDriverDesc& desc);
        ~RenderDriver();

        RenderDriver(const RenderDriver& other) = delete;
        RenderDriver(RenderDriver&& other) = delete;
        RenderDriver& operator=(const RenderDriver& other) = delete;
        RenderDriver& operator=(RenderDriver&& other) = delete;

        [[nodiscard]] NonOwningPtr<FrameContext> start_frame() noexcept;
        void end_frame(NonOwningPtr<FrameContext> frame) noexcept;
        void gpu_wait_idle();
        void submit(NonOwningPtr<GraphicsCommandList> cmd) noexcept;

        [[nodiscard]] OwningPtr<Swapchain> create_swapchain(const SwapchainDesc& desc, const Window& window);

        [[nodiscard]] NonOwningPtr<CopyCommandList> get_async_copy_cmdlist(
            uint32_t thread_idx) noexcept;
        [[nodiscard]] NonOwningPtr<ComputeCommandList> get_compute_cmdlist(
            NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept;
        [[nodiscard]] NonOwningPtr<GraphicsCommandList> get_graphics_cmdlist(
            NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept;

        [[nodiscard]] BufferHandle create_buffer(const BufferDesc& desc) noexcept;
        [[nodiscard]] ImageHandle create_image(const ImageDesc& desc) noexcept;
        [[nodiscard]] PipelineHandle create_pipeline(const GraphicsPipelineDesc& desc) noexcept;
        [[nodiscard]] PipelineHandle create_pipeline(const ComputePipelineDesc& desc) noexcept;

        [[nodiscard]] const Buffer& get_buffer_info(BufferHandle buf) const noexcept;
        [[nodiscard]] const Image& get_image_info(ImageHandle img) const noexcept;

        void destroy_buffer(BufferHandle buffer) noexcept;
        void destroy_image(ImageHandle image) noexcept;
        void destroy_pipeline(PipelineHandle pipe) noexcept;

        void internal_destroy_resource_deferred(const SwapchainDestroyPayload& payload) noexcept;
        [[nodiscard]] const Indirect& internal_get_indirect_layouts() const noexcept { return m_indirect; }

        [[nodiscard]] NonOwningPtr<ID3D12Device10> get_d3d12device() const noexcept
        { return m_device; }
        [[nodiscard]] NonOwningPtr<IDXGIFactory7> get_dxgi_factory() const noexcept
        { return m_factory; }
        [[nodiscard]] NonOwningPtr<ID3D12CommandQueue> get_direct_queue() const noexcept
        { return m_direct_queue; }

    private:

        void empty_deletion_queues(uint64_t frame) noexcept;
        void create_indirect_command_signatures() noexcept;

    private:
        template<typename T>
        struct FrameDeletionQueue
        {
            using value_type = T;

            mutable std::mutex mutex;
            struct Resource
            {
                T element;
                uint64_t frame;
            };
            std::vector<Resource> queue;
        };

        struct ResourcePools;

        template<typename T>
        union PtrOrIndex
        {
            uint64_t idx;
            NonOwningPtr<T> ptr;
        };

        Vendor m_vendor;
        const uint32_t m_thread_count;

        OwningPtr<IDXGIFactory7> m_factory;
        OwningPtr<IDXGIAdapter4> m_adapter;
        OwningPtr<ID3D12Device10> m_device;
        OwningPtr<ID3D12CommandQueue> m_direct_queue;
        OwningPtr<ID3D12CommandQueue> m_compute_queue;
        OwningPtr<ID3D12CommandQueue> m_copy_queue;
        OwningPtr<ID3D12RootSignature> m_rootsig;
        OwningPtr<ID3D12DescriptorHeap> m_cbv_srv_uav_descriptor_heap;
        OwningPtr<ID3D12DescriptorHeap> m_sampler_descriptor_heap;
        OwningPtr<ResourcePools> m_pools;
        std::atomic<uint64_t> m_frame_counter;
        Indirect m_indirect;

        FrameDeletionQueue<BufferHandle> m_buffer_deletion_queue;
        FrameDeletionQueue<ImageHandle> m_image_deletion_queue;
        FrameDeletionQueue<PipelineHandle> m_pipeline_deletion_queue;
        FrameDeletionQueue<SwapchainDestroyPayload> m_swapchain_deletion_queue;
        std::vector<FrameContext> m_frames;
    };
}
