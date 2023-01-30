#pragma once

#include <cstdint>
typedef struct HWND__* HWND;

namespace lwge
{
    class Window;
    namespace rd
    {
        class RenderDriver;
        class GraphicsCommandList;
    }
}

namespace lwge::dearimgui
{
    using wnd_proc_ptr = int64_t(*)(HWND, uint32_t, uint64_t, int64_t);
    wnd_proc_ptr get_imgui_wnd_proc() noexcept;

    void init_imgui(Window* win, rd::RenderDriver* rd) noexcept;
    void new_frame();
    void render_draw_data(rd::GraphicsCommandList* cmd);
    void shutdown_imgui() noexcept;
}
