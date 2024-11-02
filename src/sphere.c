#include <stdlib.h>
#include "Custom/sphere.h"

//--------------------------------------------------------------------------------------------------

// Bunch of functions for defining spheres
// Sphere Struct - 
// center
// radius;
// color;

//--------------------------------------------------------------------------------------------------

float random_float(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}


Vec3 random_in_unit_sphere() {
    while (1) {
        Vec3 p = vec3_random(-1.0, 1.0f);
        if (vec3_dot(p, p) < 1 && vec3_dot(p, p) != 0.0f) return vec3_normalize(p);
    }
}

Vec3 random_on_hemisphere(Vec3 normal) {
    Vec3 on_unit_sphere = random_in_unit_sphere();
    if (vec3_dot(on_unit_sphere, normal) > 0.0)
        return on_unit_sphere;
    else
        return vec3_multiply(on_unit_sphere, -1.0f);
}

Sphere create_benchmark_sphere(Vec3 center){
    Sphere sphere = {
        .center = center,
        .radius = 0.5f,
        .color = {rand() % 256, rand() % 256, rand() % 256, 255},
    };
    return sphere;
}

Sphere create_sphere(Vec3 center, float radius){
    Sphere sphere = {
        .center = center,
        .radius = radius,
        .color = {0, 0, 0},
    };
    return sphere;
}

Sphere create_random_sphere() {
    Sphere sphere = {
        .center = {random_float(-40.0f, 40.0f), random_float(-20.0f, 20.0f), random_float(-10.0, 5)},
        .radius = random_float(0.5f, 5.0f),
        .color = {rand() % 256, rand() % 256, rand() % 256, 255},
    };
    return sphere;
}

