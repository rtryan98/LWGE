#include <cstdint>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "LWGE/SystemController.hpp"
#include "LWGE/Window/InputCodes.hpp"

int32_t main(int32_t argc, const char** argv)
{
	lwge::SystemControllerDesc syscon_desc = {
		.job_system_min_threads = 8,
		.window_width = 1080,
		.window_height = 720,
		.window_title = "Hello World!"
	};
	auto syscon = lwge::SystemController(syscon_desc);
	while (syscon.is_running())
	{
		syscon.start_frame();

		syscon.end_frame();
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
