#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <agilitysdk/d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace lwge::rd
{
    using Microsoft::WRL::ComPtr;
}
