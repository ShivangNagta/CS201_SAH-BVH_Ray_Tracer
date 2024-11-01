#pragma once

#include <SDL2/SDL.h>
#include "Custom/vec3.h"


typedef struct {
    Vec3 center;
    float radius;
    SDL_Color color;
    float reflectivity;
    float transparency;
    float refractive_index;
    float diffuse;
    float specular;
    int is_light; 
} Sphere;

Sphere create_benchmark_sphere(Vec3 center);
Sphere create_random_sphere(int is_glass);
Sphere create_light_sphere();
