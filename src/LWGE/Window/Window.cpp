#include "LWGE/Window/Window.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <dxgi.h>
#include <windows.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace lwge
{
	LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		WindowData* data = reinterpret_cast<WindowData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		switch (msg)
		{
		case WM_ACTIVATE:
		{
			break;
		}
		case WM_CLOSE:
		{
			if (data)
			{
				data->alive = false;
			}
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		case WM_GETMINMAXINFO:
		{
			auto& size = reinterpret_cast<LPMINMAXINFO>(lparam)->ptMinTrackSize;
			if (data)
			{
				size.x = data->min_width;
				size.y = data->min_height;
			}
			else
			{
				size.x = 256;
				size.y = 144;
			}
			break;
		}
		case WM_SIZE:
		{
			if (data)
			{
				RECT rect;
				GetClientRect(hwnd, &rect);
				data->width = rect.right;
				data->height = rect.bottom;
			}
			break;
		}
		case WM_SYSKEYDOWN:
		{
			break;
		}
		default:
			break;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	Window::Window(uint32_t width, uint32_t height, const std::string& title)
		: Window(width, height, title, 256, 144)
	{}

	Window::Window(uint32_t width, uint32_t height, const std::string & title,
		uint32_t min_width, uint32_t min_height)
		: m_data{ .width = width, .height = height, .title = title,
			.min_width = min_width, .min_height = min_height },
		m_style{ WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX }
	{
		RECT wr = {
			.left = 0,
			.top = 0,
			.right = LONG(width),
			.bottom = LONG(height)
		};
		WNDCLASSEX wc = {
			.cbSize = sizeof(WNDCLASSEX),
			.style = 0,
			.lpfnWndProc = wnd_proc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = GetModuleHandle(NULL),
			.hIcon = LoadIcon(NULL, IDI_WINLOGO),
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hbrBackground = HBRUSH(GetStockObject(BLACK_BRUSH)),
			.lpszMenuName = nullptr,
			.lpszClassName = m_data.title.c_str(),
			.hIconSm = wc.hIcon
		};
		RegisterClassEx(&wc);
		m_hwnd = CreateWindowEx(
			0,
			wc.lpszClassName,
			wc.lpszClassName,
			m_style,
			0,
			0,
			wr.right - wr.left,
			wr.bottom - wr.top,
			nullptr,
			nullptr,
			GetModuleHandle(NULL),
			0);
		SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)(&m_data));
		ShowWindow(m_hwnd, SW_SHOWDEFAULT);
		SetForegroundWindow(m_hwnd);
		SetFocus(m_hwnd);
		m_data.alive = true;
	}

	Window::~Window()
	{
		DestroyWindow(m_hwnd);
	}

	void Window::poll_events()
	{
		MSG msg = {};
		ZeroMemory(&msg, sizeof(MSG));
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Window::toggle_borderless_fullscreen(IDXGISwapChain* swapchain)
	{
		if (m_data.fullscreen)
		{
			SetWindowLong(m_hwnd, GWL_STYLE, m_style);
			SetWindowPos(
				m_hwnd,
				HWND_NOTOPMOST,
				m_data.pos_x,
				m_data.pos_y,
				m_data.width - m_data.pos_x,
				m_data.height - m_data.pos_y,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);
			ShowWindow(m_hwnd, SW_NORMAL);
		}
		else
		{
			SetWindowLong(m_hwnd, GWL_STYLE,
				m_style & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

			RECT fs_rect = {};
			bool swapchain_output_failed = false;
			if (swapchain)
			{
				ComPtr<IDXGIOutput> output;
				swapchain_output_failed = swapchain->GetContainingOutput(&output) == S_OK;
				if (!swapchain_output_failed)
				{
					DXGI_OUTPUT_DESC desc;
					swapchain_output_failed = output->GetDesc(&desc) == S_OK;
					fs_rect = desc.DesktopCoordinates;
				}
			}
			if (!swapchain || swapchain_output_failed)
			{
				DEVMODE dm = {};
				dm.dmSize = sizeof(DEVMODE);
				EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);
				fs_rect = {
					.left = dm.dmPosition.x,
					.top = dm.dmPosition.y,
					.right = dm.dmPosition.x + static_cast<LONG>(dm.dmPelsWidth),
					.bottom = dm.dmPosition.y + static_cast<LONG>(dm.dmPelsHeight)
				};
			}
			SetWindowPos(
				m_hwnd,
				HWND_TOPMOST,
				fs_rect.left,
				fs_rect.top,
				fs_rect.right,
				fs_rect.bottom,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);
			ShowWindow(m_hwnd, SW_MAXIMIZE);
		}

		m_data.fullscreen = !m_data.fullscreen;
	}
}
