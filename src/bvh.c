#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Custom/bvh.h"
#include "Custom/ray.h"
#include "Custom/vec3.h"
#include "Custom/hit.h"
#include "Custom/constants.h"

//----------------------------------------------------------------------------------------------------

// Main implementation of Bounding Volume Hierarchies construction using Surface Area Heurestics
// Time Complexity - O( nlogn )
// Axis Aligned Bounding Boxes (AABB) are used for storing objects (child AABBs or sphers (if leaf node))
// Surface Area Heurestics (SAH) is used for finding the cost effective split plane for dividing the parent node.

//----------------------------------------------------------------------------------------------------

AABB create_empty_aabb()
{
    return (AABB){
        {INFINITY, INFINITY, INFINITY},
        {-INFINITY, -INFINITY, -INFINITY}};
}

AABB create_aabb_from_sphere(Sphere *sphere)
{
    return (AABB){
        {sphere->center.x - sphere->radius,
         sphere->center.y - sphere->radius,
         sphere->center.z - sphere->radius},
        {sphere->center.x + sphere->radius,
         sphere->center.y + sphere->radius,
         sphere->center.z + sphere->radius}};
}

AABB combine_aabb(AABB a, AABB b)
{
    return (AABB){
        {fmin(a.min.x, b.min.x),
         fmin(a.min.y, b.min.y),
         fmin(a.min.z, b.min.z)},
        {fmax(a.max.x, b.max.x),
         fmax(a.max.y, b.max.y),
         fmax(a.max.z, b.max.z)}};
}

float get_aabb_surface_area(AABB box)
{
    Vec3 dimensions = {
        box.max.x - box.min.x,
        box.max.y - box.min.y,
        box.max.z - box.min.z};
    return 2.0f * (dimensions.x * dimensions.y +
                   dimensions.y * dimensions.z +
                   dimensions.z * dimensions.x);
}

float evaluate_sah(Sphere *spheres, int start, int end, int axis, float split)
{
    int left_count = 0, right_count = 0;
    AABB left_bounds = create_empty_aabb();
    AABB right_bounds = create_empty_aabb();

    for (int i = start; i < end; i++)
    {
        float center = 0;
        switch (axis)
        {
        case 0:
            center = spheres[i].center.x;
            break;
        case 1:
            center = spheres[i].center.y;
            break;
        case 2:
            center = spheres[i].center.z;
            break;
        }

        if (center < split)
        {
            left_count++;
            left_bounds = combine_aabb(left_bounds, create_aabb_from_sphere(&spheres[i]));
        }
        else
        {
            right_count++;
            right_bounds = combine_aabb(right_bounds, create_aabb_from_sphere(&spheres[i]));
        }
    }

    float left_sa = get_aabb_surface_area(left_bounds);
    float right_sa = get_aabb_surface_area(right_bounds);

    return 0.125f + (left_count * left_sa + right_count * right_sa);
}

//----------------------------------------------------------------------------------------------------

// BVH (Bounding Volume Hierarchy) construction using a top-down approach.
// Top-Down BVH Construction :
// - Root Node Creation : contains all spheres as AABB.
// - Object Partition : AABB partition with Surface Area Heurestics (SAH).
// - Child Nodes Creation: Recursive creation and partitioning of child node.
// - Repeat Until Leaf Nodes: Partioning until each subset contains a single sphere
//                           or depth limit is reached (20 here)

//----------------------------------------------------------------------------------------------------

static void debug_aabb(AABB box, const char* label) {
    printf("%s: min=(%f,%f,%f), max=(%f,%f,%f)\n", 
           label, box.min.x, box.min.y, box.min.z,
           box.max.x, box.max.y, box.max.z);
}

BVHNode *build_bvh_node(Sphere *spheres, int start, int end, int depth)
{
    BVHNode *node = (BVHNode *)malloc(sizeof(BVHNode));
    node->bounds = create_empty_aabb();

    for (int i = start; i < end; i++)
    {
        node->bounds = combine_aabb(node->bounds, create_aabb_from_sphere(&spheres[i]));
    }

    int num_spheres = end - start;
    // debug_aabb(node->bounds, "Node bounds");


    if (num_spheres <= 1 || depth >= 40) {
        node->left = node->right = NULL;
        node->sphere = &spheres[start];
        node->sphere_count = num_spheres;
        // printf("Leaf node with %d spheres\n", num_spheres);
        return node;
    }

    float best_cost = INFINITY;
    int best_axis = 0;
    float best_split = 0;

    for (int axis = 0; axis < 3; axis++)
    {
        for (int i = 1; i < 8; i++)
        {
            float split;
            if (axis == 0)
            {
                split = node->bounds.min.x + (i / 8.0f) * (node->bounds.max.x - node->bounds.min.x);
            }
            else if (axis == 1)
            {
                split = node->bounds.min.y + (i / 8.0f) * (node->bounds.max.y - node->bounds.min.y);
            }
            else
            {
                split = node->bounds.min.z + (i / 8.0f) * (node->bounds.max.z - node->bounds.min.z);
            }

            float cost = evaluate_sah(spheres, start, end, axis, split);

            if (cost < best_cost)
            {
                best_cost = cost;
                best_axis = axis;
                best_split = split;
            }
        }
    }

    int mid = start;
    for (int i = start; i < end;)
    {
        float center = 0;
        switch (best_axis)
        {
        case 0:
            center = spheres[i].center.x;
            break;
        case 1:
            center = spheres[i].center.y;
            break;
        case 2:
            center = spheres[i].center.z;
            break;
        }

        if (center < best_split)
        {
            Sphere temp = spheres[i];
            spheres[i] = spheres[mid];
            spheres[mid] = temp;
            mid++;
            i++;
        }
        else
        {
            i++;
        }
    }

    node->left = build_bvh_node(spheres, start, mid, depth + 1);
    node->right = build_bvh_node(spheres, mid, end, depth + 1);
    node->sphere = NULL;
    node->sphere_count = 0;

    return node;
}
