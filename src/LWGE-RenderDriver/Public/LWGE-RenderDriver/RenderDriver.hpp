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

        virtual void destroy_buffer(BufferHandle buffer) noexcept = 0;
        virtual void destroy_image(ImageHandle image) noexcept = 0;
        virtual void destroy_pipeline(PipelineHandle pipe) noexcept = 0;

    protected:
        RenderDriver(const RenderDriverDesc& desc);

    protected:
        Vendor m_vendor;
        const uint32_t m_thread_count;
    };
}
