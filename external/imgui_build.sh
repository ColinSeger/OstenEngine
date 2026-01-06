#!/usr/bin/env bash

set -e
SRC="imgui_test"
OUT="imgui_build/libimgui.a"
CXXFLAGS="-O2 -std=c++17 -I$SRC -Iexternal/glfw/include/ -Lexternal/built_glfw/ -lglfw3 "

OBJS=""
for f in imgui.cpp imgui_impl_glfw.cpp imgui_impl_vulkan.cpp imgui_tables.cpp imgui_widgets.cpp imgui_draw.cpp; do
    g++ $CXXFLAGS -c "$SRC/$f" -o "${f%.cpp}.o"
    OBJS="$OBJS ${f%.cpp}.o"
done

ar rcs $OUT $OBJS
echo "Built $OUT"
