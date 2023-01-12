#pragma once
#if LWGE_BUILD_D3D12

#include "LWGE-RenderDriver/RenderDriver.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Includes.hpp"

namespace lwge::rd::d3d12
{
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

    private:
        ComPtr<IDXGIFactory7> m_factory;
        ComPtr<IDXGIAdapter4> m_adapter;
        ComPtr<ID3D12Device10> m_device;
        ComPtr<ID3D12CommandQueue> m_direct_queue;
        ComPtr<ID3D12CommandQueue> m_compute_queue;
        ComPtr<ID3D12CommandQueue> m_copy_queue;
    };
}
#endif // LWGE_BUILD_D3D12
