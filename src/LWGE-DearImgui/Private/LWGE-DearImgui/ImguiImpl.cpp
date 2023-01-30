#include "LWGE-DearImgui/ImguiImpl.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>
#include <LWGE-Window/Window.hpp>
#include <LWGE-RenderDriver/RenderDriver.hpp>
#include <LWGE-RenderDriver/CommandList.hpp>
#include <agilitysdk/d3d12.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace lwge::dearimgui
{
    wnd_proc_ptr get_imgui_wnd_proc() noexcept
    {
        return ImGui_ImplWin32_WndProcHandler;
    }

    void init_imgui(Window* win, rd::RenderDriver* rd) noexcept
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        // auto& io = ImGui::GetIO();

        ImGui_ImplWin32_Init(win->get_hwnd());

        auto increment = rd
            ->get_d3d12device()
            ->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        auto heap_handle_cpu = rd
            ->get_cbv_srv_uav_descriptor_heap()
            ->GetCPUDescriptorHandleForHeapStart();
        heap_handle_cpu.ptr +=
            uint64_t(rd::DEAR_IMGUI_DESCRIPTOR_INDEX) * uint64_t(increment);
        auto heap_handle_gpu = rd
            ->get_cbv_srv_uav_descriptor_heap()
            ->GetGPUDescriptorHandleForHeapStart();
        heap_handle_gpu.ptr +=
            uint64_t(rd::DEAR_IMGUI_DESCRIPTOR_INDEX) * uint64_t(increment);

        ImGui_ImplDX12_Init(rd->get_d3d12device(),
            rd::MAX_CONCURRENT_GPU_FRAMES,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            rd->get_cbv_srv_uav_descriptor_heap(),
            heap_handle_cpu,
            heap_handle_gpu);
    }

    void new_frame()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void render_draw_data(rd::GraphicsCommandList* cmd)
    {
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd->get_d3d12_cmdlist());
    }

    void shutdown_imgui() noexcept
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}
