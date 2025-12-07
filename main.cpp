#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_NONE
//#include "external/math_3d.h"
//#define MATH_3D_IMPLEMENTATION


#include "src/application.h"

int main()
{
    const char* name = "Vulkan Window";

    Application app = Application(800, 600, name);

    app.main_game_loop();

    // std::string result;
    std::cout << "Hello World \n";
    // std::cin >> result;

    return 0;
}
