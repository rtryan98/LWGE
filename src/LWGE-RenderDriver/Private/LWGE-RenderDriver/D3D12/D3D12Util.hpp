#pragma once
#if LWGE_BUILD_D3D12

#include "LWGE-RenderDriver/D3D12/D3D12Includes.hpp"

namespace lwge::rd::d3d12
{
    void throw_if_failed(HRESULT hr);
}

#endif
