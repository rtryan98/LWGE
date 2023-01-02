#include "LWGE/RHI/Context.hpp"
#include "LWGE/RHI/Swapchain.hpp"

#include <atomic>

namespace lwge::rhi
{
	namespace detail
	{
		struct ContextPimpl
		{
			ContextPimpl(const Window& window, uint32_t thread_count)
				: thread_count(thread_count)
			{

			}

			const uint32_t thread_count;
			std::atomic<uint64_t> total_frames = 0;
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
