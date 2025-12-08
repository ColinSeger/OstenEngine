#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_NONE
#define MATH_3D_IMPLEMENTATION
#include "external/math_3d.h"


#include "src/application.h"

int main()
{
    #ifdef _DEBUG
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif 
    
    const char* name = "Vulkan Window";

    Application app = Application(800, 600, name);

    app.main_game_loop();

    // std::string result;
    std::cout << "Hello World \n";
    // std::cin >> result;

    return 0;
}
