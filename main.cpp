#define STB_IMAGE_IMPLEMENTATION
#define GLFW_INCLUDE_NONE
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define MATH_3D_IMPLEMENTATION
#include "external/math_3d.h"

#ifdef WIN32
#include "src/win32_app.cpp"
#else
#include "src/unix_app.cpp"
#endif
int main()
{
    #ifdef _DEBUG
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
    #endif 
    
    const char* name = "Vulkan Window";

    OstenEngine app = OstenEngine(800, 600, name);

    app.main_game_loop();

    // std::string result;
    std::cout << "Hello World \n";
    // std::cin >> result;

    return 0;
}
