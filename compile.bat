g++ -o build/OstenEngine.exe main.cpp  ^
    -Iexternal/vk_include/  -Iexternal/glfw/include/ -Lexternal/built_glfw/ -Lexternal/imgui_build/ -L/VulkanSDK/1.4.328.1/Lib -lgdi32 -lglfw3  -limgui -lvulkan-1 -g
