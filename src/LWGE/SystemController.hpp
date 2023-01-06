#pragma once

#include "LWGE/RHI/Context.hpp"
#include "LWGE/Thread/JobSystem.hpp"
#include "LWGE/Window/Window.hpp"

namespace lwge
{
	struct SystemControllerDesc
	{
		uint32_t job_system_min_threads;

		uint32_t window_width;
		uint32_t window_height;
		uint32_t window_min_width = 256;
		uint32_t window_min_height = 144;
		std::string window_title;
	};

	class SystemController
	{
	public:
		SystemController(const SystemControllerDesc& desc);
		~SystemController();

		SystemController(const SystemController& other) = delete;
		SystemController(SystemController&& other) = delete;
		SystemController operator=(const SystemController& other) = delete;
		SystemController operator=(SystemController&& other) = delete;

		void start_frame();
		void end_frame();

		bool is_running() const;

		void toggle_borderless_fullscreen();

	private:
		thread::JobSystem m_job_system;
		Window m_window;
		rhi::Context m_graphics_context;
	};
}
