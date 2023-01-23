#pragma once
#if LWGE_BUILD_D3D12

typedef long HRESULT;

namespace lwge::rd::d3d12
{
    void throw_if_failed(HRESULT hr);
}

#endif
