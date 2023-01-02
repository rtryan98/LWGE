#pragma once

#include <agilitysdk/d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace lwge::rhi
{
	void throw_if_failed(HRESULT hr);
}
