//#define GLFW_INCLUDE_NONE
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>

#ifdef _WIN32
#include "src/win32_app.cpp"
#else
#include "src/unix_app.c"
#endif

#define STB_IMAGE_IMPLEMENTATION

#define MATH_3D_IMPLEMENTATION
#include "external/math_3d.h"

#define RGFW_IMPLEMENTATION
#include "external/RGFW.h"


int main()
{
    // #ifdef _DEBUG
    //     _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //     _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
    // #endif

    const char* name = "Osten Engine";

    struct OstenEngine engine = {};
    start(1920, 1080, name, &engine);

    // if(!engine.render_pipeline.device.virtual_device){
    //     return 1;
    // }
    run(&engine);

    return 0;
}
