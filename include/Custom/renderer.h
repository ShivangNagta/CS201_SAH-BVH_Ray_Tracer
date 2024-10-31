#pragma once

#include <SDL2/SDL.h>
#include "Custom/ray.h"
#include "Custom/sphere.h"
#include "Custom/bvh.h"

SDL_Color trace_ray(Ray ray, Sphere *spheres, int num_spheres, int depth, BVHNode* bvh);