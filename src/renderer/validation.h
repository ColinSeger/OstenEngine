#pragma once

#ifdef NDEBUG
    static const char* validation_layers[] = {};
#else
    static const char* validation_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
#endif
