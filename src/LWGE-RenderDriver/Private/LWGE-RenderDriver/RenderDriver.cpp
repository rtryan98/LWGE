#include "LWGE-RenderDriver/RenderDriver.hpp"
#include "LWGE-RenderDriver/CommandList.hpp"
#include "LWGE-RenderDriver/D3D12/D3D12RenderDriver.hpp"

#include <utility>

namespace lwge::rd
{
    std::unique_ptr<RenderDriver> RenderDriver::create(const RenderDriverDesc& desc)
    {
        return std::make_unique<d3d12::D3D12RenderDriver>(desc);
    }

    RenderDriver::RenderDriver(const RenderDriverDesc& desc)
        : m_vendor(Vendor::Unknown), m_thread_count(desc.thread_count)
    {}
}
