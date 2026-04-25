#!/bin/bash

set -e  # stop if any command fails

time clang -o build/OstenEngine main.c -O0 \
    -Iexternal/vk_include/  -Iexternal/glfw/include/ -Lexternal/built_glfw/ -Lexternal/ -lglfw3  -lX11 -lvulkan -g -lm

echo "Successfully Built"
