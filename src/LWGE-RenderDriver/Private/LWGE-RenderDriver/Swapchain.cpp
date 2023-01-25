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
        m_flags |= has_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

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
            .Flags = UINT(m_flags)
        };
        ComPtr<IDXGISwapChain1> sc1 = nullptr;
        throw_if_failed(dxgi_factory->CreateSwapChainForHwnd(m_driver->get_direct_queue(), m_hwnd, &sc_desc,
            nullptr, nullptr, &sc1));
        throw_if_failed(dxgi_factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));
        ComPtr<IDXGISwapChain4> sc4;
        throw_if_failed(sc1->QueryInterface(&m_swapchain));

        D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = MAX_CONCURRENT_GPU_FRAMES,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0
        };
        m_driver->get_d3d12device()->CreateDescriptorHeap(&descriptor_heap_desc,
            IID_PPV_ARGS(&m_descriptor_heap));
        m_cpu_descriptor_heap_start_address =
            m_descriptor_heap->GetCPUDescriptorHandleForHeapStart().ptr;
        m_cpu_descriptor_heap_increment =
            m_driver->get_d3d12device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        get_buffers();
        create_rtv_descriptors();
    }

    void Swapchain::get_buffers() noexcept
    {
        for (uint32_t i = 0; i < MAX_CONCURRENT_GPU_FRAMES; i++)
        {
            m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i]));
        }
    }

    void Swapchain::create_rtv_descriptors() noexcept
    {
        for (uint32_t i = 0; i < MAX_CONCURRENT_GPU_FRAMES; i++)
        {
            D3D12_CPU_DESCRIPTOR_HANDLE dst = { .ptr = get_image_descriptor_address(i) };
            D3D12_RENDER_TARGET_VIEW_DESC rtv = {
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
                .Texture2D = {
                    .MipSlice = 0,
                    .PlaneSlice = 0
                }
            };
            m_driver->get_d3d12device()->CreateRenderTargetView(m_buffers[i], &rtv, dst);
        }
    }

    Swapchain::~Swapchain()
    {
        for (auto buffer : m_buffers)
        {
            buffer->Release();
        }
        m_driver->internal_destroy_resource_deferred({ m_swapchain, m_descriptor_heap });
    }

    uint32_t Swapchain::acquire_next_image() noexcept
    {
        return m_swapchain->GetCurrentBackBufferIndex();
    }

    uint64_t Swapchain::get_image_descriptor_address(uint32_t index) const noexcept
    {
        return m_cpu_descriptor_heap_start_address
            + index * m_cpu_descriptor_heap_increment;
    }

    void Swapchain::try_resize() noexcept
    {
        DXGI_SWAP_CHAIN_DESC1 desc;
        m_swapchain->GetDesc1(&desc);
        RECT rect = {};
        GetClientRect(m_hwnd, &rect);
        uint32_t win_width = rect.right - rect.left;
        uint32_t win_height = rect.bottom - rect.top;
        if ((win_width != desc.Width || win_height != desc.Height)
            && (win_width > 0 && win_height > 0))
        {
            m_driver->gpu_wait_idle();
            for (auto buf : m_buffers)
            {
                buf->Release();
            }
            m_swapchain->ResizeBuffers(0, win_width, win_height, DXGI_FORMAT_UNKNOWN, m_flags);
            get_buffers();
            create_rtv_descriptors();
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
