#include "LWGE/RHI/Swapchain.hpp"

namespace lwge::rhi
{
	Swapchain::Swapchain(IDXGIFactory7* factory, ID3D12CommandQueue* direct_queue,
		HWND hwnd, ID3D12Device* device)
		: m_direct_queue(direct_queue), m_device(device)
	{
		BOOL has_tearing = false;
		if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&has_tearing, sizeof(has_tearing))))
		{
			has_tearing = true;
		}

		DXGI_SWAP_CHAIN_DESC1 sc_desc = {
			.Width = 0,
			.Height = 0,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = false,
			.SampleDesc = { .Count = 1, .Quality = 0 },
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = MAX_CONCURRENT_GPU_FRAMES,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = UINT(has_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0)
		};
		ComPtr<IDXGISwapChain1> sc1 = nullptr;
		throw_if_failed(factory->CreateSwapChainForHwnd(m_direct_queue, hwnd, &sc_desc,
			nullptr, nullptr, &sc1));
		throw_if_failed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
		throw_if_failed(sc1.As(&m_swapchain));

		D3D12_DESCRIPTOR_HEAP_DESC rtv_desc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			.NumDescriptors = MAX_CONCURRENT_GPU_FRAMES,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			.NodeMask = 0
		};
		m_device->CreateDescriptorHeap(&rtv_desc, IID_PPV_ARGS(&m_rtv_heap));

		get_buffers_and_rtv_descriptors();
	}

	void Swapchain::resize(uint32_t width, uint32_t height)
	{
		m_swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
		get_buffers_and_rtv_descriptors();
	}

	void Swapchain::get_buffers_and_rtv_descriptors()
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_rtv_heap->GetCPUDescriptorHandleForHeapStart();
		for (uint32_t i = 0; i < MAX_CONCURRENT_GPU_FRAMES; i++)
		{
			m_buffers[i].Reset();
			throw_if_failed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_buffers[i])));
			m_device->CreateRenderTargetView(m_buffers[i].Get(), nullptr, rtv);

			rtv.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}
	}
}
