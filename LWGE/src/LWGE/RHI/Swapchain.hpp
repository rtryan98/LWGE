#pragma once

#include <agilitysdk/d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace lwge::rhi
{
	class Swapchain
	{
	public:
		Swapchain(IDXGIFactory7* factory, ID3D12CommandQueue* direct_queue,
			HWND hwnd, ID3D12Device* device);

		IDXGISwapChain* get_dxgi_swapchain() const { return m_swapchain.Get(); }

	private:
		ID3D12CommandQueue* m_direct_queue;
		ComPtr<IDXGISwapChain4> m_swapchain;
	};
}
