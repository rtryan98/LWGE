#pragma once

#include "LWGE-RenderDriver/Resource.hpp"
#include "LWGE-RenderDriver/Swapchain.hpp"
#include <LWGE-Common/Pointer.hpp>

#include <cstdint>
#include <memory>

namespace lwge::rd
{
    struct FrameContext;
    class CopyCommandList;
    class ComputeCommandList;
    class GraphicsCommandList;

    constexpr static uint32_t MAX_CONCURRENT_GPU_FRAMES = 2;

    enum class RenderDriverAPI : uint32_t
    {
        D3D12 = 0,
        Vulkan = 1,
        Headless = 2,
        Mock = 3,
    };

    enum class Vendor : uint32_t
    {
        AMD,
        INTEL,
        NVIDIA,
        Unknown = ~0u,
    };

    struct RenderDriverDesc
    {
        RenderDriverAPI api;
        uint32_t thread_count;
    };

    class RenderDriver
    {
    public:
        static [[nodiscard]] std::unique_ptr<RenderDriver> create(const RenderDriverDesc& desc);

        virtual ~RenderDriver() = default;

        virtual [[nodiscard]] NonOwningPtr<FrameContext> start_frame() noexcept = 0;
        virtual void end_frame(NonOwningPtr<FrameContext> frame) noexcept = 0;
        virtual void gpu_wait_idle() = 0;

        virtual [[nodiscard]] NonOwningPtr<CopyCommandList> get_async_copy_cmdlist(
            uint32_t thread_idx) noexcept = 0;
        virtual [[nodiscard]] NonOwningPtr<ComputeCommandList> get_compute_cmdlist(
            NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept = 0;
        virtual [[nodiscard]] NonOwningPtr<GraphicsCommandList> get_graphics_cmdlist(
            NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept = 0;

        virtual [[nodiscard]] BufferHandle create_buffer(const BufferDesc& desc) noexcept = 0;
        virtual [[nodiscard]] ImageHandle create_image(const ImageDesc& desc) noexcept = 0;
        virtual [[nodiscard]] PipelineHandle create_pipeline(const GraphicsPipelineDesc& desc) noexcept = 0;
        virtual [[nodiscard]] PipelineHandle create_pipeline(const ComputePipelineDesc& desc) noexcept = 0;

        [[nodiscard]] RenderDriverAPI get_api() const noexcept { return m_api; }

    protected:
        RenderDriver(const RenderDriverDesc& desc);

    protected:
        RenderDriverAPI m_api;
        Vendor m_vendor;
        const uint32_t m_thread_count;
    };
}
