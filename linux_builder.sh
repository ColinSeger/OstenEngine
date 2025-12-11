#!/bin/bash

set -e  # stop if any command fails

time g++ -o build/OstenEngine main.cpp  \
    -Iexternal/glfw/include/ -Lexternal/built_glfw/ -lglfw3 -Lexternal/ -limgui  -lX11 -Iexternal/vk_include/ -lvulkan -g

echo "Successfully Built"
