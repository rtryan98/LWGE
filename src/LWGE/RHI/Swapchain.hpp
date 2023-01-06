#pragma once

#include <array>
#include <agilitysdk/d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "LWGE/RHI/D3D12Util.hpp"

namespace lwge::rhi
{
	class Swapchain
	{
	public:
		Swapchain(IDXGIFactory7* factory, ID3D12CommandQueue* direct_queue,
			HWND hwnd, ID3D12Device* device);

		IDXGISwapChain4* get_dxgi_swapchain() const { return m_swapchain.Get(); }
		void resize(uint32_t width, uint32_t height);

	private:
		void get_buffers_and_rtv_descriptors();

	private:
		ID3D12CommandQueue* m_direct_queue;
		ID3D12Device* m_device;
		ComPtr<IDXGISwapChain4> m_swapchain;
		ComPtr<ID3D12DescriptorHeap> m_rtv_heap;
		std::array<ComPtr<ID3D12Resource>, MAX_CONCURRENT_GPU_FRAMES> m_buffers;
	};
}
