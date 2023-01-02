#include "LWGE/RHI/Swapchain.hpp"

namespace lwge::rhi
{
	Swapchain::Swapchain(IDXGIFactory7* factory, ID3D12CommandQueue* direct_queue,
		HWND hwnd, ID3D12Device* device)
		: m_direct_queue(direct_queue)
	{
		BOOL has_tearing = false;
		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&has_tearing, sizeof(has_tearing))))
		{
			has_tearing = true;
		}
		hwnd; device;

	}
}
