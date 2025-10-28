#pragma once
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>

class Instance
{
    VkInstance instance;

public:
    Instance(const char* name);
    ~Instance();
};