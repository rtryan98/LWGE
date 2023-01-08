#pragma once

#include <agilitysdk/d3d12.h>
#include <cstdint>
#include <wrl.h>

namespace lwge::rhi
{
	using Microsoft::WRL::ComPtr;

	constexpr static uint32_t MAX_CONCURRENT_GPU_FRAMES = 2;

	void throw_if_failed(HRESULT hr);
}
