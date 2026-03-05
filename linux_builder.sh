#!/bin/bash

set -e  # stop if any command fails

time g++ -o build/OstenEngine main.cpp -fwhole-program -O0 -Wall \
    -Iexternal/vk_include/  -Iexternal/glfw/include/ -Lexternal/built_glfw/ -Lexternal/ -lglfw3  -limgui  -lX11 -lvulkan -g

echo "Successfully Built"
