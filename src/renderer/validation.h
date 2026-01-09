#pragma once

#ifdef NDEBUG
    static const char* validation_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    const int validation_amount = 0;
#else
    static const char* validation_layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    const int validation_amount = sizeof(validation_layers) / sizeof(validation_layers[0]);
#endif
