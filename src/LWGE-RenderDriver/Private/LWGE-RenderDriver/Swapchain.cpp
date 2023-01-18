#include "LWGE-RenderDriver/Swapchain.hpp"
#include "LWGE-RenderDriver/RenderDriver.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Swapchain.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12RenderDriver.hpp"

namespace lwge::rd
{
    std::unique_ptr<Swapchain> Swapchain::create(const SwapchainDesc& desc,
        RenderDriver* driver, Window* window)
    {
        switch (driver->get_api())
        {
#if LWGE_BUILD_D3D12
        case RenderDriverAPI::D3D12:
            return std::make_unique<d3d12::D3D12Swapchain>(
                desc, static_cast<d3d12::D3D12RenderDriver*>(driver), window);
#endif
        case RenderDriverAPI::Vulkan:
            std::unreachable();
        case RenderDriverAPI::Mock:
            std::unreachable();
        case RenderDriverAPI::Headless:
            std::unreachable();
        default:
            std::unreachable();
        }
    }
}
