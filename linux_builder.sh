#!/bin/bash

set -e  # stop if any command fails

time g++ -o build/OstenEngine main.cpp  \
    src/engine/entity_manager/entity_manager.cpp \
    src/engine/entity_manager/components.cpp\
    src/engine/transform.cpp \
    -Iexternal/glfw/include/ -Lexternal/built_glfw/ -lglfw3 -Lexternal/ -limgui  -lX11 -Iexternal/vk_include/ -lvulkan -g

echo "Successfully Built"
