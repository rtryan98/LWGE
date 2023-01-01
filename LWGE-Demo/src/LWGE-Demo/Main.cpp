#if LWGE_TARGET_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#include <cstdint>

int32_t main(int32_t argc, const char** argv)
{
	return 0;
}

#if LWGE_TARGET_WINDOWS
int32_t WINAPI WinMain(
	_In_ HINSTANCE /*hInstance*/,
	_In_opt_ HINSTANCE /*hPrevInstance*/,
	_In_ LPSTR /*lpCmdLine*/,
	_In_ int32_t /*nShowCmd*/)
{
	return main(0, nullptr);
}
#endif
