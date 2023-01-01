#include <cstdint>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "LWGE/Window/Window.hpp"

int32_t main(int32_t argc, const char** argv)
{
	auto window = lwge::Window(1280, 720, "Hello World");
	while (window.get_window_data().alive)
	{
		window.poll_events();
	}
	return 0;
}

int32_t WINAPI WinMain(
	_In_ HINSTANCE /*hInstance*/,
	_In_opt_ HINSTANCE /*hPrevInstance*/,
	_In_ LPSTR /*lpCmdLine*/,
	_In_ int32_t /*nShowCmd*/)
{
	return main(0, nullptr);
}
