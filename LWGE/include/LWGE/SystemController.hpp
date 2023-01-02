#pragma once

#include "LWGE/RHI/Context.hpp"
#include "LWGE/Window/Window.hpp"

namespace lwge
{
	class SystemController
	{
	public:
		SystemController();

		void toggle_borderless_fullscreen();

	private:
		Window m_window;
		rhi::Context m_graphics_context;
	};
}
