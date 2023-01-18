#include <LWGE-RenderDriver/RenderDriver.hpp>
#include <LWGE-Thread/JobSystem.hpp>
#include <LWGE-Window/Window.hpp>

int32_t main(int32_t, const char*)
{
    auto js = lwge::thread::JobSystem(8);
    lwge::rd::RenderDriverDesc rd_desc = {
        .api = lwge::rd::RenderDriverAPI::D3D12,
        .thread_count = 1
    };
    lwge::WindowDesc win_desc = {
        .width = 1280,
        .height = 720,
        .title = "LWGE-Demo"
    };
    auto window = lwge::Window(win_desc);
    auto rd = lwge::rd::RenderDriver::create(rd_desc);
    lwge::rd::SwapchainDesc sc_desc = {
        .vsync = false
    };
    auto sc = lwge::rd::Swapchain::create(sc_desc, rd.get(), &window);
    while (window.get_window_data().alive)
    {
        window.poll_events();
        auto frame = rd->start_frame();

        rd->end_frame(frame);
    }
    return 0;
}
