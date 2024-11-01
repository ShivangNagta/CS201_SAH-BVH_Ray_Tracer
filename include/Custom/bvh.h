#pragma once

#include "Custom/vec3.h"
#include "Custom/sphere.h"
#include "Custom/ray.h"

typedef struct AABB {
    Vec3 min;
    Vec3 max;
} AABB;

typedef struct BVHNode {
    AABB bounds;
    struct BVHNode* left;
    struct BVHNode* right;
    Sphere* sphere;
    int sphere_count;
} BVHNode;


AABB create_empty_aabb();
AABB create_aabb_from_sphere(Sphere* sphere);
AABB combine_aabb(AABB a, AABB b);
float get_aabb_surface_area(AABB box);
float evaluate_sah(Sphere* spheres, int start, int end, int axis, float split);
BVHNode* build_bvh_node(Sphere* spheres, int start, int end, int depth);

