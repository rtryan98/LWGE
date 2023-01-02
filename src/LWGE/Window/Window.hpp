#pragma once

#include <cstdint>
#include <string>

typedef struct HWND__* HWND;
struct IDXGISwapChain;

namespace lwge
{
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
	};

	class Window
	{
	public:
		Window(uint32_t width, uint32_t height, const std::string& title);
		Window(uint32_t width, uint32_t height, const std::string& title,
			uint32_t min_width, uint32_t min_height);
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
