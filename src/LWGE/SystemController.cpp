#include "LWGE/SystemController.hpp"

namespace lwge
{
	void SystemController::toggle_borderless_fullscreen()
	{
		m_window.toggle_borderless_fullscreen(m_graphics_context.get_swapchain());
	}
}
