g++ -o build/OstenEngine main.cpp  ^
    external/imgui_test/imgui.cpp ^
    external/imgui_test/imgui_impl_glfw.cpp ^
    external/imgui_test/imgui_impl_vulkan.cpp ^
    external/imgui_test/imgui_tables.cpp ^
    external/imgui_test/imgui_widgets.cpp ^
    external/imgui_test/imgui_draw.cpp ^
    src/application.cpp ^
    src/renderer/instance/vulkan/instance.cpp ^
    src/renderer/device/vulkan/device.cpp ^
    src/renderer/swap_chain/vulkan/swap_chain.cpp ^
    src/renderer/render_data/vulkan/render_data.cpp ^
    src/renderer/render_pipeline.cpp ^
    src/renderer/texture/vulkan/texture.cpp ^
    src/renderer/model_loader/model_loader.cpp ^
    src/engine/entity_manager/entity_manager.cpp ^
    src/engine/transform.cpp ^
    src/debugger/debugger.cpp ^
    -Iexternal/glfw/include/ -Iexternal/vk_include/ -g