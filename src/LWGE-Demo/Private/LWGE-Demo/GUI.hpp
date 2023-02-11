#pragma once

namespace ax::NodeEditor
{
    struct EditorContext;
}
namespace lwge
{
    class Window;
    namespace rd
    {
        class RenderDriver;
        class GraphicsCommandList;
    }
}

class GUI
{
public:
    GUI(lwge::Window* window, lwge::rd::RenderDriver* render_driver);
    ~GUI();

    void process();
    void render(lwge::rd::GraphicsCommandList* cmd);

private:
    void main_menu();

private:
    ax::NodeEditor::EditorContext* m_node_ctx;
    bool m_first_frame = true;
};
