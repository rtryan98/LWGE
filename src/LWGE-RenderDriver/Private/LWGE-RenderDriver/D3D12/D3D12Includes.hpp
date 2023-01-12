#pragma once

#if LWGE_BUILD_D3D12
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <agilitysdk/d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace lwge::rd::d3d12
{
    using Microsoft::WRL::ComPtr;
}
#endif // LWGE_BUILD_D3D12
