// #include "transform.h"
#pragma once
#include "../../external/math_3d.h"

struct Vector2{
    float x = 0;
    float y = 0;
};

struct Transform{
    vec3_t position = {0.0f, 0.0f, 0.0f};
    vec3_t rotation = {0.0f, 0.0f, 0.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};
};

namespace Transformations
{
    mat4_t get_model_matrix(Transform transform){//This should be a for loop inside renderer probably
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


    vec3_t forward_vector(Transform transform){
        return {
            cos(transform.rotation.z) * sin(transform.rotation.y),
            -sin(transform.rotation.z),
            cos(transform.rotation.z) * cos(transform.rotation.y)
        };
    }

    vec3_t right_vector(Transform transform){
        return
        {
            cos(transform.rotation.y),
            0,
            -sin(transform.rotation.y)
        };
    }

    vec3_t up_vector(Transform transform){
        vec3_t f = forward_vector(transform);
        vec3_t r = right_vector(transform);
        vec3_t forward = {f.x, f.y, f.z};
        vec3_t right = {r.x, r.y, r.z};
        vec3_t res = v3_cross(forward, right);
        // return cross(forward_vector(transform), right_vector(transform));

        return {res.x, res.y, res.z};
    }
}
