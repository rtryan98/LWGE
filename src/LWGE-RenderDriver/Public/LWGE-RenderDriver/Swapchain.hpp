#pragma once

#include <LWGE-Common/Pointer.hpp>

#include <cstdint>
#include <memory>

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
        static [[nodiscard]] std::unique_ptr<Swapchain> create(const SwapchainDesc& desc,
            NonOwningPtr<RenderDriver> driver, NonOwningPtr<Window> window);

        virtual ~Swapchain() = default;

        virtual void resize(uint32_t width, uint32_t height) noexcept = 0;

    protected:
        Swapchain(const SwapchainDesc& desc) noexcept
            : m_vsync_enabled(desc.vsync)
        {};

    protected:
        bool m_vsync_enabled;
    };
}
