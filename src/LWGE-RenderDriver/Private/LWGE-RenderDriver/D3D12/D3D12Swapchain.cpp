#include "LWGE-RenderDriver/D3D12/D3D12Swapchain.hpp"
#if LWGE_BUILD_D3D12

#include "LWGE-Window/Window.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12RenderDriver.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Util.hpp"

namespace lwge::rd::d3d12
{
    D3D12Swapchain::D3D12Swapchain(const SwapchainDesc& desc, D3D12RenderDriver* driver,
            Window* window)
        : Swapchain(desc),
        m_driver(driver),
        m_swapchain(nullptr),
        m_hwnd(window->get_hwnd())
    {
        auto dxgi_factory = m_driver->get_dxgi_factory();

        BOOL has_tearing = false;
        if (SUCCEEDED(dxgi_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
            &has_tearing, sizeof(has_tearing))))
        {
            has_tearing = true;
        }

        DXGI_SWAP_CHAIN_DESC1 sc_desc = {
            .Width = 0,
            .Height = 0,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .Stereo = false,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = MAX_CONCURRENT_GPU_FRAMES,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = UINT(has_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0)
        };
        ComPtr<IDXGISwapChain1> sc1 = nullptr;
        throw_if_failed(dxgi_factory->CreateSwapChainForHwnd(driver->get_direct_queue(), m_hwnd, &sc_desc,
            nullptr, nullptr, &sc1));
        throw_if_failed(dxgi_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
        throw_if_failed(sc1.As(&m_swapchain));
    }

    D3D12Swapchain::~D3D12Swapchain()
    {
        m_driver->destroy_resource_deferred(m_swapchain);
    }

    void D3D12Swapchain::resize(uint32_t width, uint32_t height) noexcept
    {
        m_swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    }
}

#endif // LWGE_BUILD_D3D12
