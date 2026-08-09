#pragma once
#include <stdint.h>
#include <math.h>
//TODO Look into just doing a #Define IMPLEMENTATION

enum ConstNumbers{
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN
};

enum ConstByteSizes{
    KILOBYTES = 1024,
    MEGABYTES = KILOBYTES * 1024,
    GIGABYTES = MEGABYTES * 1024
};

#define u8 uint8_t
#define i8 int8_t
#define u32 uint32_t
#define i32 int32_t

#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286

static inline float sine_f(float value){
    float result = sin(value);//TODO My own sine instead of C default
    return result;
}

static inline float cosine_f(float value){
    float result = cos(value);//TODO My own cos instead of C default
    return result;
}

static inline float power_to_f(float value, float power){
    float result = pow(value, power);//TODO My own pow instead of C default
    return result;
}

typedef union vec2 {
    struct {
        float x;
        float y;
    };
    float cords[2];
} vec2;

static inline vec2 vec2_add(vec2 value_a, vec2 value_b) {
    vec2 result = {
        .x = value_a.x + value_b.y,
        .y = value_b.y + value_b.y
    };
    return result;
}

static inline vec2 vec2_sub(vec2 value_a, vec2 value_b) {
    vec2 result = {
        .x = value_a.x - value_b.y,
        .y = value_b.y - value_b.y
    };
    return result;
}

static inline vec2 vec2_multiply(vec2 value_a, vec2 value_b) {
    vec2 result = {
        .x = value_a.x * value_b.y,
        .y = value_b.y * value_b.y
    };
    return result;
}

static inline vec2 vec2_divide(vec2 value_a, vec2 value_b) {
    vec2 result = {
        .x = value_a.x / value_b.y,
        .y = value_b.y / value_b.y
    };
    return result;
}

typedef union vec3 {
    struct {
        float x;
        float y;
        float z;
    };
    float cords[3];
} vec3;

static inline vec3 vec3_f(float x, float y, float z){
    vec3 result = (vec3){{x, y, z}};
    return result;
}

typedef union vec4 {
    struct {
        float x;
        float y;
        float z;
        float u;//TODO Remember what last one is called
    };
    float cords[4];
} vec4;
