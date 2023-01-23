#include "LWGE-RenderDriver/Swapchain.hpp"
#include "LWGE-RenderDriver/RenderDriver.hpp"
#include "LWGE-RenderDriver/D3D12Includes.hpp"
#include "LWGE-RenderDriver/Util.hpp"
#include <LWGE-Window/Window.hpp>

namespace lwge::rd
{
    Swapchain::Swapchain(const SwapchainDesc& desc, RenderDriver& driver,
            const Window& window) noexcept
        : m_vsync_enabled(desc.vsync),
        m_driver(&driver),
        m_hwnd(window.get_hwnd())
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
        throw_if_failed(dxgi_factory->CreateSwapChainForHwnd(m_driver->get_direct_queue(), m_hwnd, &sc_desc,
            nullptr, nullptr, &sc1));
        throw_if_failed(dxgi_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
        ComPtr<IDXGISwapChain4> sc4;
        throw_if_failed(sc1->QueryInterface(&m_swapchain));

        for (uint32_t i = 0; i < MAX_CONCURRENT_GPU_FRAMES; i++)
        {
            m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i]));
        }
    }

    Swapchain::~Swapchain()
    {
        for (auto buffer : m_buffers)
        {
            buffer->Release();
        }
        m_driver->destroy_resource_deferred(m_swapchain);
    }

    void Swapchain::try_resize() noexcept
    {
        DXGI_SWAP_CHAIN_DESC1 desc;
        m_swapchain->GetDesc1(&desc);
        RECT rect = {};
        GetClientRect(m_hwnd, &rect);
        uint32_t win_width = rect.right - rect.left;
        uint32_t win_height = rect.bottom - rect.top;
        if (win_width != desc.Width || win_height != desc.Height)
        {
            m_driver->gpu_wait_idle();
            m_swapchain->ResizeBuffers(0, win_width, win_height, DXGI_FORMAT_UNKNOWN, 0);
        }
    }

    void Swapchain::present() noexcept
    {
        if (m_vsync_enabled)
        {
            m_swapchain->Present(1, 0);
        }
        else
        {
            m_swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
        }
    }
}
