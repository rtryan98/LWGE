#include "LWGE-RenderDriver/D3D12/D3D12Util.hpp"
#if LWGE_BUILD_D3D12
#include "LWGE-RenderDriver/D3D12/D3D12Includes.hpp"
#include <exception>

namespace lwge::rd::d3d12
{
    void throw_if_failed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw std::exception();
        }
    }
}

#endif
