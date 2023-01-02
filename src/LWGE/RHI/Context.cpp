#include "LWGE/RHI/Context.hpp"
#include "LWGE/RHI/Swapchain.hpp"
#include "LWGE/Window/Window.hpp"

#include <atomic>

namespace lwge::rhi
{
	namespace detail
	{
		ComPtr<IDXGIFactory7> create_dxgi_factory()
		{

		}

		struct ContextPimpl
		{
			ContextPimpl(const Window& window, uint32_t thread_count)
				: thread_count(thread_count),
				factory(create_dxgi_factory()),
				device(),
				direct_queue(),
				swapchain(factory.Get(), direct_queue.Get(), window.get_hwnd(), device.Get())
			{

			}

			const uint32_t thread_count;
			std::atomic<uint64_t> total_frames = 0;
			ComPtr<IDXGIFactory7> factory;
			ComPtr<ID3D12Device10> device;
			ComPtr<ID3D12CommandQueue> direct_queue;
			Swapchain swapchain;
		};
	}

	Context::Context(const Window& window, uint32_t thread_count)
		: m_pimpl(new detail::ContextPimpl(window, thread_count))
	{}

	Context::~Context()
	{
		delete m_pimpl;
	}

	IDXGISwapChain* Context::get_swapchain() const
	{
		return m_pimpl->swapchain.get_dxgi_swapchain();
	}
}
