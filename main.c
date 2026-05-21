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


// int main(){
//     const char* name = "Osten Engine";

//     struct OstenEngine engine = {};
//     create_osten_engine(1920, 1080, name, &engine);

//     run(&engine);

//     return 0;
// }
