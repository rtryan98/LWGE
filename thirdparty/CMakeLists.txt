ADD_LIBRARY(IMGUI
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_dx12.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_dx12.h)
TARGET_INCLUDE_DIRECTORIES(IMGUI PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
ADD_LIBRARY(IMGUI-NODE-EDITOR
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/crude_json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/crude_json.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_bezier_math.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_bezier_math.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_canvas.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_canvas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_extra_math.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_extra_math.inl
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_node_editor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_node_editor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_node_editor_api.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_node_editor_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor/imgui_node_editor_internal.inl)
TARGET_INCLUDE_DIRECTORIES(IMGUI-NODE-EDITOR PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui-node-editor)
TARGET_LINK_LIBRARIES(IMGUI-NODE-EDITOR PUBLIC IMGUI)
