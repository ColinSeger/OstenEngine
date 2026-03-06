# OstenEngine

Is a work in progress game "engine" made in C++ and Vulkan, tried to use minimal dependencies for this project while still keeping 
the development within a resonable time.

To build this project you need to be able to run the vkcube command, info on that can be found at https://vulkan.lunarg.com/ 


## Build

### Windows

#### Prerequisites

You will need Cmake to be able to build the build files You can find out more on that here https://cmake.org/.

Vulkan is required for this project to run, you can check if you have Vulkan dev tools installed by running the console command 
"vkcube" and if a cube saying lunarg shows up you have Vulkan tools already, otherwhise go to https://vulkan.lunarg.com/ and install the Vulkan tools.

The compiler I used for testing on Windows was their MSVC compiler but I belive it should be able to compile using other compilers that can be used with Cmake.

On windows it should just be to run cmake --build command in the Engine folder if that does not work you might not have a compiler installed,
it could also be that you need to install GLFW on you system.

### Linux

-Requirements
- Cmake compatible C++ compiler
- Vulkan

Slightly more advanced than windows but still quite simple, all you should need is vulkan developer packages installed on your system and a
C++ compiler. First you will need to compile GLFW into a library and the same for ImGui, for ImGui there is a helper script in the external folder (imgui_build.sh).

When both of them are libraries you should just need to run (linux_builder.sh) and it should work.

Though if you are not using g++ you would need to change to your desired builder in (linux_builder.sh)

### Additional Step

The engine at the moment does not support figuring out file paths automatically and instead you will need to either run it in the engine folder or move required items to the same folder as the executable

* Means that files in this folder are needed
Folder paths that need to exist in running directory:
src/renderer/shaders/* 
assets/debug_assets/*
