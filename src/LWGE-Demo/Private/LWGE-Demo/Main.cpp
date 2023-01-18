#include <LWGE-RenderDriver/RenderDriver.hpp>
#include <LWGE-RenderDriver/CommandList.hpp>
#include <LWGE-Thread/JobSystem.hpp>
#include <LWGE-Window/Window.hpp>
#include <LWGE-Window/InputCodes.hpp>

int32_t main(int32_t, const char*)
{
    auto js = lwge::thread::JobSystem(15);
    lwge::rd::RenderDriverDesc rd_desc = {
        .api = lwge::rd::RenderDriverAPI::D3D12,
        .thread_count = js.get_worker_thread_cnt() + 1
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
        const auto& input = window.get_window_data().input;
        if (input.is_key_clicked(lwge::KeyCode::KeyF11))
        {
            window.toggle_borderless_fullscreen();
        }
        auto cmd = rd->get_graphics_cmdlist(frame, js.get_thread_idx());
        cmd->begin_recording();

        cmd->end_recording();
        rd->end_frame(frame);
    }
    return 0;
}
