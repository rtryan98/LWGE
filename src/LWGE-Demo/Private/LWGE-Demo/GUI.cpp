#include "GUI.hpp"

#include <LWGE-DearImgui/ImguiImpl.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor.h>

GUI::GUI(lwge::Window* window, lwge::rd::RenderDriver* render_driver)
{
    using namespace ImGui;

    lwge::dearimgui::init_imgui(window, render_driver);

    auto& imgui_io = ImGui::GetIO();
    imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imgui_io.ConfigDockingAlwaysTabBar = false;
    IM_DELETE(imgui_io.Fonts);
    imgui_io.Fonts = IM_NEW(ImFontAtlas);
    ImFontConfig font_cfg = {};
    font_cfg.OversampleH = 4;
    font_cfg.OversampleV = 4;
    font_cfg.PixelSnapH = false;
    imgui_io.Fonts->AddFontFromFileTTF(
        "res/font/roboto_mono/static/RobotoMono-Regular.ttf",
        16.0f,
        &font_cfg);
    imgui_io.Fonts->Build();

    auto& imgui_style = ImGui::GetStyle();

    imgui_style.Alpha;
    imgui_style.DisabledAlpha;
    imgui_style.WindowPadding;
    imgui_style.WindowRounding = 0.0f;
    imgui_style.WindowBorderSize = 0.0f;
    imgui_style.WindowMinSize;
    imgui_style.WindowTitleAlign;
    imgui_style.WindowMenuButtonPosition = ImGuiDir_None;
    imgui_style.ChildRounding = 0.0f;
    imgui_style.ChildBorderSize = 0.0f;
    imgui_style.PopupRounding;
    imgui_style.PopupBorderSize;
    imgui_style.FramePadding;
    imgui_style.FrameRounding;
    imgui_style.FrameBorderSize;
    imgui_style.ItemSpacing;
    imgui_style.ItemInnerSpacing;
    imgui_style.CellPadding;
    imgui_style.TouchExtraPadding;
    imgui_style.IndentSpacing;
    imgui_style.ColumnsMinSpacing;
    imgui_style.ScrollbarSize;
    imgui_style.ScrollbarRounding;
    imgui_style.GrabMinSize;
    imgui_style.GrabRounding;
    imgui_style.LogSliderDeadzone;
    imgui_style.TabRounding;
    imgui_style.TabBorderSize;
    imgui_style.TabMinWidthForCloseButton;
    imgui_style.ColorButtonPosition;
    imgui_style.ButtonTextAlign;
    imgui_style.SelectableTextAlign;
    imgui_style.DisplayWindowPadding;
    imgui_style.DisplaySafeAreaPadding;
    imgui_style.MouseCursorScale;
    imgui_style.AntiAliasedLines;
    imgui_style.AntiAliasedLinesUseTex;
    imgui_style.AntiAliasedFill;
    imgui_style.CurveTessellationTol;
    imgui_style.CircleTessellationMaxError;

    m_node_ctx = ax::NodeEditor::CreateEditor();
    ax::NodeEditor::SetCurrentEditor(m_node_ctx);
    auto& node_editor_style = ax::NodeEditor::GetStyle();
    node_editor_style.NodeRounding = 0.0f;
    node_editor_style.PinRadius = 1.0f;
    ax::NodeEditor::SetCurrentEditor(nullptr);
}

GUI::~GUI()
{
    ax::NodeEditor::DestroyEditor(m_node_ctx);
    lwge::dearimgui::shutdown_imgui();
}

void GUI::process()
{
    lwge::dearimgui::new_frame();
    auto viewport = ImGui::GetMainViewport();
    if (!(viewport->Size.x > .0f) || !(viewport->Size.y > .0f))
    {
        ImGui::Render();
        return;
    }

    main_menu();

    constexpr static const char* node_editor_panel = "panel:node_editor";
    constexpr static const char* toolbox_panel_right = "panel:toolbox:right";
    constexpr static const char* toolbox_panel_left = "panel:toolbox:left";
    constexpr static const char* viewport_panel = "panel:viewport";

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    auto dockspace = ImGui::DockSpaceOverViewport(viewport);
    if (m_first_frame)
    {
        ImGui::DockBuilderRemoveNode(dockspace);
        ImGui::DockBuilderAddNode(dockspace,
            ImGuiDockNodeFlags_DockSpace
            | ImGuiDockNodeFlags_NoResizeX
            | ImGuiDockNodeFlags_NoResizeY
            | ImGuiDockNodeFlags_CentralNode);
        ImGui::DockBuilderSetNodeSize(dockspace, viewport->Size);
        auto dock_id_down = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Down, 0.45f, nullptr, &dockspace);
        auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.2f, nullptr, &dockspace);
        auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Left, 0.2f, nullptr, &dockspace);

        ImGui::DockBuilderDockWindow(node_editor_panel, dock_id_down);
        ImGui::DockBuilderDockWindow(toolbox_panel_right, dock_id_right);
        ImGui::DockBuilderDockWindow(toolbox_panel_left, dock_id_left);
        ImGui::DockBuilderDockWindow(viewport_panel, dockspace);

        ImGui::DockBuilderFinish(dockspace);
    }
    ImGui::PopStyleColor();

    ImGuiWindowClass wnd_class_no_tab = {};
    wnd_class_no_tab.DockNodeFlagsOverrideSet =
        ImGuiDockNodeFlags_NoTabBar
        | ImGuiDockNodeFlags_NoDocking
        | ImGuiDockNodeFlags_NoWindowMenuButton;

    ImGuiWindowClass wnd_class_no_dock = {};
    wnd_class_no_dock.DockNodeFlagsOverrideSet =
        ImGuiDockNodeFlags_NoDocking
        | ImGuiDockNodeFlags_NoWindowMenuButton;

    ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0);
    ImGui::SetNextWindowClass(&wnd_class_no_dock);
    ImGui::Begin(node_editor_panel, nullptr,
        ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoTitleBar);
    ax::NodeEditor::SetCurrentEditor(m_node_ctx);
    ax::NodeEditor::Begin("Terrain Graph");

    uint32_t id = 0;
    ax::NodeEditor::BeginNode(id++);
    ImGui::Text("Result");
    ax::NodeEditor::BeginPin(id++, ax::NodeEditor::PinKind::Input);
    ImGui::Text("-> Input");
    ax::NodeEditor::EndPin();
    static float val = 0.0f;
    ImGui::PushItemWidth(125.0f);
    ImGui::DragFloat("Factor", &val, float(1/255), 0.0f, 1.0f, "%.3f");
    ax::NodeEditor::EndNode();
    ax::NodeEditor::End();
    ax::NodeEditor::SetCurrentEditor(nullptr);
    ImGui::End();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::SetNextWindowClass(&wnd_class_no_tab);
    ImGui::Begin(viewport_panel, nullptr,
        ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoTitleBar);

    ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
    ImGui::Text("Width: %f, Height: %f", ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
    ImGui::Text("Offset X: %f, Offset Y: %f", ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);

    ImGui::End();
    ImGui::PopStyleColor(1);

    ImGui::SetNextWindowClass(&wnd_class_no_tab);
    ImGui::Begin(toolbox_panel_right, nullptr,
        ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoTitleBar);
    ImGui::End();
    ImGui::SetNextWindowClass(&wnd_class_no_tab);
    ImGui::Begin(toolbox_panel_left, nullptr,
        ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoTitleBar);
    ImGui::End();
    ImGui::PopStyleColor();

    m_first_frame = false;
}

void GUI::render(lwge::rd::GraphicsCommandList* cmd)
{
    ImGui::Render();
    lwge::dearimgui::render_draw_data(cmd);
}

void GUI::main_menu()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New")) {}
            if (ImGui::MenuItem("Open", "Ctrl+O")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
