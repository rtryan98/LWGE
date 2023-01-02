#pragma once

#include <cstdint>

struct IDXGISwapChain;

namespace lwge
{
	class Window;
}

namespace lwge::rhi
{
	namespace detail
	{
		struct ContextPimpl;
	}

	class Context
	{
	public:
		Context(const Window& window, uint32_t thread_count);
		~Context();

		[[nodiscard]] IDXGISwapChain* get_swapchain() const;

	private:
		detail::ContextPimpl* m_pimpl;
	};
}
