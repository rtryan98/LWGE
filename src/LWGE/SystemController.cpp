#include "LWGE/SystemController.hpp"

namespace lwge
{
	SystemController::SystemController(const SystemControllerDesc& desc)
		: m_job_system(desc.job_system_min_threads),
		m_window(desc.window_width, desc.window_height,
			desc.window_title, desc.window_min_width, desc.window_min_height),
		m_graphics_context(m_window, m_job_system.get_worker_thread_cnt())
	{}

	SystemController::~SystemController()
	{
		m_job_system.stop();
	}

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
