#include "LWGE-RenderDriver/Swapchain.hpp"
#include "LWGE-RenderDriver/RenderDriver.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12Swapchain.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12RenderDriver.hpp"

namespace lwge::rd
{
    std::unique_ptr<Swapchain> Swapchain::create(const SwapchainDesc& desc,
        NonOwningPtr<RenderDriver> driver, NonOwningPtr<Window> window)
    {
        return std::make_unique<d3d12::D3D12Swapchain>(
            desc, static_cast<d3d12::D3D12RenderDriver*>(driver), window);
    }
}
