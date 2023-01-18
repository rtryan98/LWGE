#pragma once
#if LWGE_BUILD_D3D12

#include "LWGE-RenderDriver/Swapchain.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Includes.hpp"

namespace lwge::rd::d3d12
{
    class D3D12RenderDriver;

    class D3D12Swapchain : public Swapchain
    {
    public:
        D3D12Swapchain(const SwapchainDesc& desc, D3D12RenderDriver* driver, Window* window);
        virtual ~D3D12Swapchain() override;
        virtual void resize(uint32_t width, uint32_t height) noexcept override;

    private:
        NonOwningPtr<D3D12RenderDriver> m_driver;
        ComPtr<IDXGISwapChain4> m_swapchain;
        HWND m_hwnd;
    };
}

#endif // LWGE_BUILD_D3D12
