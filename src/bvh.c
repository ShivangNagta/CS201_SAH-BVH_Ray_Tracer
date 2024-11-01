#include <stdlib.h>
#include "Custom/bvh.h"
#include "Custom/ray.h"
#include <math.h>

AABB create_empty_aabb() {
    return (AABB){
        {INFINITY, INFINITY, INFINITY},
        {-INFINITY, -INFINITY, -INFINITY}
    };
}

AABB create_aabb_from_sphere(Sphere* sphere) {
    return (AABB){
        {sphere->center.x - sphere->radius, 
         sphere->center.y - sphere->radius, 
         sphere->center.z - sphere->radius},
        {sphere->center.x + sphere->radius, 
         sphere->center.y + sphere->radius, 
         sphere->center.z + sphere->radius}
    };
}

AABB combine_aabb(AABB a, AABB b) {
    return (AABB){
        {fmin(a.min.x, b.min.x), 
         fmin(a.min.y, b.min.y), 
         fmin(a.min.z, b.min.z)},
        {fmax(a.max.x, b.max.x), 
         fmax(a.max.y, b.max.y), 
         fmax(a.max.z, b.max.z)}
    };
}

float get_aabb_surface_area(AABB box) {
    Vec3 dimensions = {
        box.max.x - box.min.x,
        box.max.y - box.min.y,
        box.max.z - box.min.z
    };
    return 2.0f * (dimensions.x * dimensions.y + 
                   dimensions.y * dimensions.z + 
                   dimensions.z * dimensions.x);
}


// BVH building functions
float evaluate_sah(Sphere* spheres, int start, int end, int axis, float split) {
    int left_count = 0, right_count = 0;
    AABB left_bounds = create_empty_aabb();
    AABB right_bounds = create_empty_aabb();
    
    for (int i = start; i < end; i++) {
        float center = 0;
        switch(axis) {
            case 0: center = spheres[i].center.x; break;
            case 1: center = spheres[i].center.y; break;
            case 2: center = spheres[i].center.z; break;
        }
        
        if (center < split) {
            left_count++;
            left_bounds = combine_aabb(left_bounds, create_aabb_from_sphere(&spheres[i]));
        } else {
            right_count++;
            right_bounds = combine_aabb(right_bounds, create_aabb_from_sphere(&spheres[i]));
        }
    }
    
    float left_sa = get_aabb_surface_area(left_bounds);
    float right_sa = get_aabb_surface_area(right_bounds);
    
    return 0.125f + (left_count * left_sa + right_count * right_sa);
}

BVHNode* build_bvh_node(Sphere* spheres, int start, int end, int depth) {
    BVHNode* node = (BVHNode*)malloc(sizeof(BVHNode));
    node->bounds = create_empty_aabb();
    
    // Create bounding box for all spheres in this node
    for (int i = start; i < end; i++) {
        node->bounds = combine_aabb(node->bounds, create_aabb_from_sphere(&spheres[i]));
    }
    
    int num_spheres = end - start;
    
    // Leaf node criteria
    if (num_spheres <= 1 || depth >= 20) {
        node->left = node->right = NULL;
        node->sphere = &spheres[start];
        node->sphere_count = num_spheres;
        return node;
    }
    
    // Find best split using SAH
    float best_cost = INFINITY;
    int best_axis = 0;
    float best_split = 0;
    
    // Try each axis
    for (int axis = 0; axis < 3; axis++) {
        // Sample several split positions
        for (int i = 1; i < 8; i++) {
            float split = node->bounds.min.x + 
                (i / 8.0f) * (node->bounds.max.x - node->bounds.min.x);
            float cost = evaluate_sah(spheres, start, end, axis, split);
            
            if (cost < best_cost) {
                best_cost = cost;
                best_axis = axis;
                best_split = split;
            }
        }
    }
    
    // Partition spheres based on best split
    int mid = start;
    for (int i = start; i < end;) {
        float center = 0;
        switch(best_axis) {
            case 0: center = spheres[i].center.x; break;
            case 1: center = spheres[i].center.y; break;
            case 2: center = spheres[i].center.z; break;
        }
        
        if (center < best_split) {
            // Swap spheres[i] and spheres[mid]
            Sphere temp = spheres[i];
            spheres[i] = spheres[mid];
            spheres[mid] = temp;
            mid++;
            i++;
        } else {
            i++;
        }
    }
    
    // Create child nodes
    node->left = build_bvh_node(spheres, start, mid, depth + 1);
    node->right = build_bvh_node(spheres, mid, end, depth + 1);
    node->sphere = NULL;
    node->sphere_count = 0;
    
    return node;
}


