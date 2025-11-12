#!/bin/bash

echo "Hello, World!"

set -e  # stop if any command fails


GLFW_DIR="../glfw"
BUILD_DIR="external/built_glfw"

# === PREPARE BUILD DIR ===
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# === COMPILE ALL NECESSARY GLFW SOURCE FILES ===
gcc -c \
  $GLFW_DIR/src/context.c \
  $GLFW_DIR/src/init.c \
  $GLFW_DIR/src/input.c \
  $GLFW_DIR/src/monitor.c \
  $GLFW_DIR/src/vulkan.c \
  $GLFW_DIR/src/window.c \
  $GLFW_DIR/src/egl_context.c \
  $GLFW_DIR/src/osmesa_context.c \
  $GLFW_DIR/src/platform.c \
  $GLFW_DIR/src/posix_thread.c \
  $GLFW_DIR/src/posix_time.c \
  $GLFW_DIR/src/x11_init.c \
  $GLFW_DIR/src/x11_monitor.c \
  $GLFW_DIR/src/x11_window.c \
  $GLFW_DIR/src/glx_context.c \
  $GLFW_DIR/src/linux_joystick.c \
  -I../$GLFW_DIR/include \
  -fPIC


ar rcs libglfw3.a *.o

echo "✅ Built static library: $BUILD_DIR/libglfw3.a"

cd "../../"

gcc main.cpp  \
    external/vk_include/vulkan/vulkan.h\
    external/imgui/imgui.cpp \
    external/imgui/backends/imgui_impl_glfw.cpp \
    external/imgui/backends/imgui_impl_vulkan.cpp \
    src/application.cpp \
    src/renderer/instance/vulkan/instance.cpp \
    src/renderer/device/vulkan/device.cpp \
    src/renderer/swap_chain/vulkan/swap_chain.cpp \
    src/renderer/render_data/vulkan/render_data.cpp \
    src/renderer/render_pipeline.cpp \
    src/renderer/texture/vulkan/texture.cpp \
    src/renderer/model_loader/model_loader.cpp \
    -L-external/built_glfw/libglfw3.a -I/external/vk_include -lvulkan -o build/OstenEngine

./build/OstenEngine