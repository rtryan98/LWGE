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
		struct ContextImpl;

		constexpr static uint32_t MAX_CONCURRENT_GPU_FRAMES = 2;
	}

	class Context
	{
	public:
		Context(const Window& window, uint32_t thread_count);
		~Context();

		[[nodiscard]] IDXGISwapChain* get_swapchain() const;

	private:
		detail::ContextImpl* m_pimpl;
	};
}
