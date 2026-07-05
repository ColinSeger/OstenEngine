
#ifdef _WIN32
#include "src/win32_app.cpp"
#else
#include "src/unix_app.c"
#endif

#define STB_IMAGE_IMPLEMENTATION

#define RGFW_IMPLEMENTATION
#include "external/RGFW.h"
