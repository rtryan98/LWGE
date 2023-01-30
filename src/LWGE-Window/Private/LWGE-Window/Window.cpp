#include "LWGE-Window/Window.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <dxgi.h>
#include <windows.h>
#include <dwmapi.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace lwge
{
    LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        WindowData* data = reinterpret_cast<WindowData*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (data && data->wnd_proc_callback)
            if (data->wnd_proc_callback(hwnd, msg, wparam, lparam))
                return true;

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
        case WM_NCACTIVATE:
        {
            break;
        }
        case WM_NCCALCSIZE:
        {
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

    Window::Window(const WindowDesc& desc)
        : m_data{ .width = desc.width, .height = desc.height, .title = desc.title,
            .min_width = desc.min_width, .min_height = desc.min_height,
            .wnd_proc_callback = desc.wnd_proc_callback},
        m_style{ WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX }
    {
        RECT wr = {
            .left = LONG((GetSystemMetrics(SM_CXSCREEN) - m_data.width) / 2),
            .top = LONG((GetSystemMetrics(SM_CYSCREEN) - m_data.height) / 2),
            .right = LONG(m_data.width),
            .bottom = LONG(m_data.height)
        };
        AdjustWindowRectEx(&wr, m_style, FALSE, WS_EX_TOOLWINDOW);
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
            wr.left,
            wr.top,
            wr.right,
            wr.bottom,
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
        m_data.input.m_keys_last_frame = m_data.input.m_keys_current_frame;
        GetKeyboardState(m_data.input.m_keys_current_frame.data());
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void Window::toggle_borderless_fullscreen()
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
            HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX monitor_info = {};
            GetMonitorInfo(monitor, &monitor_info);
            DEVMODE dm = {};
            dm.dmSize = sizeof(DEVMODE);
            EnumDisplaySettings(monitor_info.szDevice, ENUM_CURRENT_SETTINGS, &dm);
            fs_rect = {
                .left = dm.dmPosition.x,
                .top = dm.dmPosition.y,
                .right = dm.dmPosition.x + static_cast<LONG>(dm.dmPelsWidth),
                .bottom = dm.dmPosition.y + static_cast<LONG>(dm.dmPelsHeight)
            };
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
