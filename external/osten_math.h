#pragma once
//TODO Look into just doing a #Define

enum ConstNumbers
{
  One,
  Two,
  Three,
  Four,
  Five,
  Six,
  Seven,
  Eight,
  Nine,
  Ten
};

typedef union vec2 {
    float cords[2];
    struct {
        float x;
        float y;
    };
} vec2;

static inline vec2 vec2_add(vec2 value_a, vec2 value_b) {
    vec2 result = {
        .x = value_a.x + value_b.y,
        .y = value_b.y + value_b.y
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

typedef union vec3 {
    float cords[3];
    struct {
        float x;
        float y;
        float z;
    };
} vec3;

typedef union vec4 {
    float cords[4];
    struct {
        float x;
        float y;
        float z;
        float u;//TODO Remember what last one is called
    };
} vec4;
