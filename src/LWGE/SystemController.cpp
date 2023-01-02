#include "LWGE/SystemController.hpp"

namespace lwge
{
	SystemController::SystemController(const SystemControllerDesc& desc)
		: m_window(desc.window_width, desc.window_height,
			desc.window_title, desc.window_min_width, desc.window_min_height),
		m_graphics_context(m_window, 1)
	{}

	void SystemController::start_frame()
	{
		m_window.poll_events();
	}

	void SystemController::end_frame()
	{

	}

	bool SystemController::is_running() const
	{
		return m_window.get_window_data().alive;
	}

	void SystemController::toggle_borderless_fullscreen()
	{
		m_window.toggle_borderless_fullscreen(m_graphics_context.get_swapchain());
	}
}
