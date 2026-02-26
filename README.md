# OstenEngin

Is a work in progress game "engine" made in C++ and Vulkan, tried to use minimal dependencies for this project while still keeping 
the development withing resonable time.


To build this project you need to be able to run the vkcube command, info on that can be found at https://vulkan.lunarg.com/ 

### Windows

On windows it should just be to run cmake --build command in the Engine folder if that does not work you might not have a compiler installed,
it could also be that you need to install GLFW on you system.

### Linux

Slightly more advanced than windows but still quite simple, all you should need is vulkan developer packages installed on your system and a
C++ compiler. First you will need to compile GLFW into a library and the same for ImGui, for ImGui there is a helper script in the external folder (imgui_build.sh).

When both of them are libraries you should just need to run (linux_builder.sh) and it should work.
