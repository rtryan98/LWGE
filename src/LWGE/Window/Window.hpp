#pragma once

#include <cstdint>
#include <string>

#include "LWGE/Window/Input.hpp"

typedef struct HWND__* HWND;
struct IDXGISwapChain;

namespace lwge
{
	struct WindowDesc
	{
		uint32_t width;
		uint32_t height;
		uint32_t min_width = 256;
		uint32_t min_height = 144;
		std::string title;
	};

	struct WindowData
	{
		uint32_t width;
		uint32_t height;
		std::string title;
		uint32_t min_width;
		uint32_t min_height;
		uint32_t pos_x;
		uint32_t pos_y;
		bool alive;
		bool fullscreen;
		Input input;
	};

	class Window
	{
	public:
		Window(const WindowDesc& desc);
		~Window();

		void poll_events();
		void toggle_borderless_fullscreen(IDXGISwapChain* swapchain);
		[[nodiscard]] HWND get_hwnd() const { return m_hwnd; }
		[[nodiscard]] const WindowData& get_window_data() const { return m_data; }

	private:
		HWND m_hwnd = nullptr;
		WindowData m_data = {};
		int32_t m_style = 0;
	};
}
