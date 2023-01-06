#pragma once

#include <atomic>
#include <cstdint>

#include "lwge/RHI/D3D12Util.hpp"
#include "LWGE/RHI/Swapchain.hpp"

namespace lwge
{
	class Window;
}

namespace lwge::rhi
{
	class Context
	{
	public:
		Context(const Window& window, uint32_t thread_count);
		~Context();

		void start_frame();
		void end_frame();

		void await_gpu_idle();

		[[nodiscard]] IDXGISwapChain* get_swapchain() const;

	private:
		const uint32_t m_thread_count;
		std::atomic<uint64_t> m_total_frames = 0;
		ComPtr<IDXGIFactory7> m_factory;
		ComPtr<IDXGIAdapter4> m_adapter;
		ComPtr<ID3D12Device10> m_device;
		ComPtr<ID3D12CommandQueue> m_direct_queue;
		ComPtr<ID3D12CommandQueue> m_compute_queue;
		ComPtr<ID3D12CommandQueue> m_copy_queue;
		Swapchain m_swapchain;
	};
}
