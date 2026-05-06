#!/bin/bash

set -e  # stop if any command fails

time clang -o build/OstenEngine main.c -O0 -Wall \
    -Iexternal/vk_include/ -Lexternal/ -lX11 -lvulkan -lGL -lm -lXrandr -g -lm

echo "Successfully Built"
