#pragma once

#include "LWGE-RenderDriver/CommandList.hpp"
#include "LWGE-Common/Pointer.hpp"

#include <cstdint>
#include <memory>

namespace lwge::rd
{
    enum class RenderDriverAPI : uint32_t
    {
        D3D12,
        Mock,
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

    struct FrameContext;

    class RenderDriver
    {
    public:
        static [[nodiscard]] std::unique_ptr<RenderDriver> create(const RenderDriverDesc& desc);

        virtual ~RenderDriver() = 0;

        virtual [[nodiscard]] NonOwningPtr<FrameContext> start_frame() noexcept = 0;
        virtual void end_frame(NonOwningPtr<FrameContext> frame) noexcept = 0;
        virtual void gpu_wait_idle() = 0;

        virtual [[nodiscard]] NonOwningPtr<CopyCommandList> get_async_copy_cmdlist(
            uint32_t thread_idx) noexcept = 0;
        virtual [[nodiscard]] NonOwningPtr<ComputeCommandList> get_compute_cmdlist(
            NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept = 0;
        virtual [[nodiscard]] NonOwningPtr<GraphicsCommandList> get_graphics_cmdlist(
            NonOwningPtr<FrameContext> frame, uint32_t thread_idx) noexcept = 0;

    protected:
        RenderDriver(const RenderDriverDesc& desc);
        [[nodiscard]] CommandListToken make_cmdlist_token() const noexcept;

    protected:
        RenderDriverAPI m_api;
        Vendor m_vendor;
        const uint32_t m_thread_count;
    };
}
