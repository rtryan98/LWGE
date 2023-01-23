#pragma once

#include <LWGE-Common/Pointer.hpp>

#include "LWGE-RenderDriver/Constants.hpp"

#include <array>
#include <cstdint>

typedef struct HWND__* HWND;
struct IDXGISwapChain4;
struct ID3D12Resource;

namespace lwge
{
    class Window;
}

namespace lwge::rd
{
    class RenderDriver;

    struct SwapchainDesc
    {
        bool vsync;
    };

    class Swapchain
    {
    public:
        Swapchain(const SwapchainDesc& desc, RenderDriver& driver,
            const Window& window) noexcept;
        ~Swapchain();

        Swapchain(const Swapchain& other) = delete;
        Swapchain(Swapchain&& other) = delete;
        Swapchain& operator=(const Swapchain& other) = delete;
        Swapchain& operator=(Swapchain&& other) = delete;

        void try_resize() noexcept;
        void present() noexcept;

    private:
        bool m_vsync_enabled;
        NonOwningPtr<RenderDriver> m_driver;
        OwningPtr<IDXGISwapChain4> m_swapchain = {};
        std::array<OwningPtr<ID3D12Resource>, MAX_CONCURRENT_GPU_FRAMES> m_buffers;
        HWND m_hwnd = {};
    };
}
