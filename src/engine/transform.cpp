// #include "transform.h"
#pragma once
#include "../../external/math_3d.h"


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

struct Transform
{
    Vector3 position = {0.0f, 0.0f, 0.0f};
    Vector3 rotation = {0.0f, 0.0f, 0.0f};
    Vector3 scale = {1.0f, 1.0f, 1.0f};
};

namespace Transformations
{
    mat4_t  get_model_matrix(Transform transform);

    Vector3 forward_vector(Transform transform);

    Vector3 right_vector(Transform transform);

    Vector3 up_vector(Transform transform);
}
mat4_t Transformations::get_model_matrix(Transform transform)//This should be a for loop inside renderer probably
{
    mat4_t model = mat4(
        1,  0,  0,  0,
        0,  1,  0,  0,
        0,  0,  1,  0,
        transform.position.x,  transform.position.y, transform.position.z,  1
    );
    model = m4_mul(model, m4_rotation_x(transform.rotation.x));
    model = m4_mul(model, m4_rotation_y(transform.rotation.y));
    model = m4_mul(model, m4_rotation_z(transform.rotation.z));
    model = m4_mul(model, m4_scaling({transform.scale.x, transform.scale.y, transform.scale.z}));
    return model;
}


Vector3 Transformations::forward_vector(Transform transform)
{
    return {
        cos(transform.rotation.z) * sin(transform.rotation.y),
        -sin(transform.rotation.z),
        cos(transform.rotation.z) * cos(transform.rotation.y)
    };
}

Vector3 Transformations::right_vector(Transform transform)
{
    return
    {
        cos(transform.rotation.y),
        0,
        -sin(transform.rotation.y)
    };
}

Vector3 Transformations::up_vector(Transform transform)
{
    Vector3 f = forward_vector(transform);
    Vector3 r = right_vector(transform);
    vec3_t forward = {f.x, f.y, f.z};
    vec3_t right = {r.x, r.y, r.z};
    vec3_t res = v3_cross(forward, right);
    // return cross(forward_vector(transform), right_vector(transform));

    return Vector3{res.x, res.y, res.z};
}
