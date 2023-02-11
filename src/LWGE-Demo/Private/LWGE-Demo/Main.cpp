#include <LWGE-RenderDriver/RenderDriver.hpp>
#include <LWGE-RenderDriver/CommandList.hpp>
#include <LWGE-Thread/JobSystem.hpp>
#include <LWGE-Window/Window.hpp>
#include <LWGE-Window/InputCodes.hpp>
#include <LWGE-DearImgui/ImguiImpl.hpp>

#include <memory>

#include "GUI.hpp"

void transition_swapchain_to_render_target_state(
    lwge::rd::GraphicsCommandList* cmd,
    lwge::rd::Swapchain* sc,
    uint32_t sc_img_index)
{
    using namespace lwge;

    cmd->set_render_target(sc, sc_img_index);
    auto pre_clear_image_barrier = std::to_array({
        rd::ImageBarrier {
            .sync_before = rd::SyncStage::None,
            .sync_after = rd::SyncStage::RenderTarget,
            .access_before = rd::Access::NoAccess,
            .access_after = rd::Access::RenderTarget,
            .layout_before = rd::ImageLayout::Undefined,
            .layout_after = rd::ImageLayout::RenderTarget,
            .swapchain = sc,
            .swapchain_image_index = sc_img_index,
            .subresources = {
                .index_or_first_mip_level = 0xffffffff,
                .mip_level_count = 0,
                .first_array_slice = 0,
                .array_slice_count = 0,
                .first_plane = 0,
                .plane_count = 0
            },
            .discard = true
        }
        });
    auto pre_clear_barrier_group = std::to_array({
        rd::BarrierGroup {
            .type = rd::BarrierType::Image,
            .image_barriers = pre_clear_image_barrier
        }
        });
    cmd->barrier(pre_clear_barrier_group);
}

void transition_swapchain_to_present_state(
    lwge::rd::GraphicsCommandList* cmd,
    lwge::rd::Swapchain* sc,
    uint32_t sc_img_index)
{
    using namespace lwge;

    auto post_clear_image_barrier = std::to_array({
        rd::ImageBarrier {
            .sync_before = rd::SyncStage::RenderTarget,
            .sync_after = rd::SyncStage::None,
            .access_before = rd::Access::RenderTarget,
            .access_after = rd::Access::NoAccess,
            .layout_before = rd::ImageLayout::RenderTarget,
            .layout_after = rd::ImageLayout::Present,
            .swapchain = sc,
            .swapchain_image_index = sc_img_index,
            .subresources = {
                .index_or_first_mip_level = 0xffffffff,
                .mip_level_count = 0,
                .first_array_slice = 0,
                .array_slice_count = 0,
                .first_plane = 0,
                .plane_count = 0
            },
            .discard = false
        }
        });
    auto post_clear_barrier_group = std::to_array({
        rd::BarrierGroup {
            .type = rd::BarrierType::Image,
            .image_barriers = post_clear_image_barrier
        }
        });
    cmd->barrier(post_clear_barrier_group);
}

void render_frame(
    lwge::rd::GraphicsCommandList* cmd,
    GUI* gui,
    lwge::rd::Swapchain* sc,
    uint32_t sc_img_index)
{
    using namespace lwge;

    transition_swapchain_to_render_target_state(
        cmd, sc, sc_img_index);
    std::array<float, 4> rgba = { 0.0f, 0.0f, 0.0f, 1.0f };
    cmd->clear_render_target(sc, sc_img_index, rgba);
    gui->render(cmd);
    transition_swapchain_to_present_state(
        cmd, sc, sc_img_index);
}

int32_t main(int32_t, const char*)
{
    using namespace lwge;

    auto js = thread::JobSystem(1);
    rd::RenderDriverDesc rd_desc = {
        .thread_count = js.get_worker_thread_cnt() + 1
    };
    WindowDesc win_desc = {
        .width = 1920,
        .height = 1080,
        .title = "LWGE-Demo",
        .wnd_proc_callback = dearimgui::get_imgui_wnd_proc()
    };
    auto window = Window(win_desc);
    auto rd = std::make_unique<rd::RenderDriver>(rd_desc);
    rd::SwapchainDesc sc_desc = {
        .vsync = false
    };
    auto sc = std::unique_ptr<rd::Swapchain>(rd->create_swapchain(sc_desc, window));
    auto gui = std::make_unique<GUI>(&window, rd.get());

    while (window.get_window_data().alive)
    {
        window.poll_events();
        auto frame = rd->start_frame();
        sc->try_resize();
        const auto& input = window.get_window_data().input;
        if (input.is_key_clicked(KeyCode::KeyF11))
        {
            window.toggle_borderless_fullscreen();
        }
        gui->process();

        auto sc_img_index = sc->acquire_next_image();
        auto cmd = rd->get_graphics_cmdlist(frame, thread::JobSystem::get_thread_idx());
        cmd->begin_recording();
        render_frame(cmd, gui.get(), sc.get(), sc_img_index);
        cmd->end_recording();
        rd->submit(cmd);
        sc->present();
        rd->end_frame(frame);
    }
    js.stop();
    return 0;
}
