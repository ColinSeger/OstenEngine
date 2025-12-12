@echo off
setlocal enabledelayedexpansion

set SRC=imgui_test
set OUT=imgui_build\libimgui.a
set CXXFLAGS=-O2 -std=c++17 -I%SRC% -Iglfw\include\ -Ivk_include\ -Lbuilt_glfw\ -lglfw3

set OBJS=

for %%f in (imgui.cpp imgui_impl_glfw.cpp imgui_impl_vulkan.cpp imgui_tables.cpp imgui_widgets.cpp imgui_draw.cpp) do (
    g++ %CXXFLAGS% -c "%SRC%\%%f" -o "%%~nf.o"
    set OBJS=!OBJS! %%~nf.o
)

ar rcs %OUT% %OBJS%
echo Built %OUT%

endlocal
