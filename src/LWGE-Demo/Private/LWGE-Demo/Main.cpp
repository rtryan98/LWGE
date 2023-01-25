#include <LWGE-RenderDriver/RenderDriver.hpp>
#include <LWGE-RenderDriver/CommandList.hpp>
#include <LWGE-Thread/JobSystem.hpp>
#include <LWGE-Window/Window.hpp>
#include <LWGE-Window/InputCodes.hpp>

#include <memory>

int32_t main(int32_t, const char*)
{
    auto js = lwge::thread::JobSystem(1);
    lwge::rd::RenderDriverDesc rd_desc = {
        .thread_count = js.get_worker_thread_cnt() + 1
    };
    lwge::WindowDesc win_desc = {
        .width = 1280,
        .height = 720,
        .title = "LWGE-Demo"
    };
    auto window = lwge::Window(win_desc);
    auto rd = std::make_unique<lwge::rd::RenderDriver>(rd_desc);
    lwge::rd::SwapchainDesc sc_desc = {
        .vsync = false
    };
    auto sc = std::unique_ptr<lwge::rd::Swapchain>(rd->create_swapchain(sc_desc, window));
    while (window.get_window_data().alive)
    {
        window.poll_events();
        auto frame = rd->start_frame();
        sc->try_resize();
        auto sc_img_index = sc->acquire_next_image();
        const auto& input = window.get_window_data().input;
        if (input.is_key_clicked(lwge::KeyCode::KeyF11))
        {
            window.toggle_borderless_fullscreen();
        }
        auto cmd = rd->get_graphics_cmdlist(frame, 0);

        cmd->begin_recording();
        cmd->set_render_target(sc.get(), sc_img_index);
        std::array<float, 4> rgba = { 1.0f, 0.5f, 0.0f, 1.0f };
        cmd->clear_render_target(sc.get(), sc_img_index, rgba);
        cmd->end_recording();
        rd->submit(cmd);
        sc->present();
        rd->end_frame(frame);
    }
    return 0;
}
