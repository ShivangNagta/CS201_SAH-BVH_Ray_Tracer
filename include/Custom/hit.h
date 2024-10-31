#pragma once

#include "vec3.h"
#include "ray.h"
#include "sphere.h"

typedef struct {
    float t;
    Vec3 point;
    Vec3 normal;
    int hit_something;
    Sphere *object;
} HitRecord;

HitRecord ray_sphere_intersect(Ray ray, Sphere *sphere);

