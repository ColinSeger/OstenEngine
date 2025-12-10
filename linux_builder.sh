#!/bin/bash

set -e  # stop if any command fails

g++ -o build/OstenEngine main.cpp  \
    src/renderer/instance/vulkan/instance.cpp \
    src/renderer/descriptors/descriptors.cpp\
    src/renderer/device/vulkan/device.cpp \
    src/renderer/swap_chain/vulkan/swap_chain.cpp \
    src/renderer/render_data/vulkan/render_data.cpp \
    src/renderer/render_pipeline.cpp \
    src/renderer/texture/vulkan/texture.cpp \
    src/renderer/model_loader/model_loader.cpp \
    src/engine/entity_manager/entity_manager.cpp \
    src/engine/entity_manager/components.cpp\
    src/engine/transform.cpp \
    src/debugger/debugger.cpp \
    src/editor/file_explorer/file_explorer.cpp \
    -Iexternal/glfw/include/ -Lexternal/built_glfw/ -lglfw3 -Lexternal/ -limgui  -lX11 -Iexternal/vk_include/ -lvulkan -g

echo "Successfully Built"
