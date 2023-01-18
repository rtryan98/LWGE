#include "LWGE-RenderDriver/RenderDriver.hpp"
#include "LWGE-RenderDriver/CommandList.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12RenderDriver.hpp"

#include <utility>

namespace lwge::rd
{
    std::unique_ptr<RenderDriver> RenderDriver::create(const RenderDriverDesc& desc)
    {
        switch (desc.api)
        {
#if LWGE_BUILD_D3D12
        case RenderDriverAPI::D3D12: {
            return std::make_unique<d3d12::D3D12RenderDriver>(desc);
        }
#endif
        case RenderDriverAPI::Mock: {
            return nullptr; // TODO: implement mock
        }
        default: {
            std::unreachable();
        }
        }
    }

    RenderDriver::RenderDriver(const RenderDriverDesc& desc)
        : m_api(desc.api), m_vendor(Vendor::Unknown), m_thread_count(desc.thread_count)
    {}
}
