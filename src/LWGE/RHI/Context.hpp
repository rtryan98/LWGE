#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <vector>

#include "LWGE/RHI/D3D12Util.hpp"
#include "LWGE/RHI/CommandListRecycler.hpp"
#include "LWGE/RHI/Swapchain.hpp"

namespace lwge
{
	class Window;
}

namespace lwge::rhi
{
	struct FrameContext
	{
#pragma warning(push)
#pragma warning(disable : 4324) // Structure was padded due to alignment specifier
		struct alignas(std::hardware_destructive_interference_size) ThreadData
		{
			CommandListRecycler direct_queue_cmd_list_recycler;
			CommandListRecycler compute_queue_cmd_list_recycler;
		};
#pragma warning(pop)
		ComPtr<ID3D12Fence1> direct_queue_fence;
		ComPtr<ID3D12Fence1> compute_queue_fence;

		uint64_t frame_fence_val;
		std::vector<ThreadData> thread_data;
	};

	class Context
	{
	public:
		Context(const Window& window, uint32_t thread_count);
		~Context();

		FrameContext* start_frame();
		void end_frame(FrameContext* frame_context);

		void await_gpu_idle();

		[[nodiscard]] IDXGISwapChain* get_swapchain() const;

	private:
		void wait_on_frame_context(FrameContext* frame);
		void start_frame_context(FrameContext* frame);
		void check_for_swapchain_resize();

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
		std::array<FrameContext, MAX_CONCURRENT_GPU_FRAMES> m_frame_contexts;
	};
}
