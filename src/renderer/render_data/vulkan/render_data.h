#pragma once
#include <cstdint>

enum Direction : uint8_t{
    X = 0,
    Y = 1,
    Z = 2
};

struct Vector2{
    float x = 0;
    float y = 0;
};

struct Vector3{
    float x = 0;
    float y = 0;
    float z = 0;

    Vector3 operator+(const Vector3 add){
        return {
            x + add.x,
            y + add.y,
            z + add.z
        };
    }
    Vector3& operator-=(const Vector3 subtract){
        x -= subtract.x;
        y -= subtract.y;
        z -= subtract.z;
        return *this;
    }
    Vector3& operator+=(const Vector3 add){
        x += add.x;
        y += add.y;
        z += add.z;
        return *this;
    }
    Vector3& operator*(const float mul){
        x *= mul;
        y *= mul;
        z *= mul;
        return *this;
    }
};
