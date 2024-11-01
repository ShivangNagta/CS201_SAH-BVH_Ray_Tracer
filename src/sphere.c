#include <stdlib.h>
#include "Custom/sphere.h"

// Random functions
float random_float(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

Vec3 random_in_unit_sphere() {
    while (1) {
        Vec3 p = {
            random_float(-1, 1),
            random_float(-1, 1),
            random_float(-1, 1)
        };
        if (vec3_dot(p, p) < 1) return p;
    }
}

Sphere create_benchmark_sphere(Vec3 center){
    Sphere sphere = {
        .center = center,
        .radius = 5.0f,
        .color = {rand() % 256, rand() % 256, rand() % 256, 255},
        .reflectivity = random_float(0.0f, 0.5f),
        .transparency =  0.0f,
        .refractive_index = 1.0f,
        .diffuse = random_float(0.1f, 0.9f),
        .specular = random_float(1.0f, 32.0f),
        .is_light = 0
    };
    return sphere;
}

Sphere create_random_sphere(int is_glass) {
    Sphere sphere = {
        .center = {random_float(-5, 5), random_float(0.5, 5), random_float(-5, 5)},
        .radius = random_float(0.5f, 1.5f),
        .color = {rand() % 256, rand() % 256, rand() % 256, 255},
        .reflectivity = is_glass ? 0.9f : random_float(0.0f, 0.5f),
        .transparency = is_glass ? 0.9f : 0.0f,
        .refractive_index = is_glass ? 1.5f : 1.0f,
        .diffuse = is_glass ? 0.1f : random_float(0.1f, 0.9f),
        .specular = is_glass ? 32.0f : random_float(1.0f, 32.0f),
        .is_light = 0
    };
    return sphere;
}


Sphere create_light_sphere() {
    Sphere light = {
        .center = {15, 4, -2},
        .radius = 10.0f,
        .color = {255, 255, 200, 255}, 
        .reflectivity = 0.0f,
        .transparency = 0.0f,
        .refractive_index = 1.0f,
        .diffuse = 0.0f,
        .specular = 0.0f,
        .is_light = 1
    };
    return light;
}