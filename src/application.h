#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <cassert>

class Application
{

    GLFWwindow* main_window = nullptr;

public:

    Application(const int width, const int height, const char* name);

    ~Application();
    void main_game_loop();
};