#include "Custom/hit.h"
#include "Custom/vec3.h"
#include "Custom/constants.h"
#include "Custom/bvh.h"
#include <math.h>

//--------------------------------------------------------------------------------------------------

// ray_sphere_intersect() - Returns the Hitrecord for the ray intersecting with the sphere
// Hitrecords is a struct containing distance 
// - t (distance of the hitpoint from the camera initially, then the distance of the next hitpoint from the current hitpoint)
// - point (position of the current hitpoint)
// - normal (normal to the surface at hitpoint - point)
// - hit_something ( bool in terms of int)
// - sphere* (pointer to current sphere object)

//--------------------------------------------------------------------------------------------------

HitRecord ray_sphere_intersect(Ray ray, Sphere *sphere) {
    HitRecord rec = {0};
    Vec3 oc = vec3_sub(ray.origin, sphere->center);
    float a = vec3_dot(ray.direction, ray.direction);
    float b = 2.0f * vec3_dot(oc, ray.direction);
    float c = vec3_dot(oc, oc) - sphere->radius * sphere->radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant > 0) {
        float t = (-b - sqrt(discriminant)) / (2.0f * a);
        if (t > EPSILON) {
            rec.hit_something = 1;
            rec.t = t;
            rec.point = vec3_add(ray.origin, vec3_multiply(ray.direction, t));
            rec.normal = vec3_normalize(vec3_sub(rec.point, sphere->center));
            rec.object = sphere;
            return rec;
        }
    }
    return rec;
}

//--------------------------------------------------------------------------------------------------

// ray_aabb_intersect() - Returns the 1 if the given AABB is hit with the ray otherwise 0
// Performs slab test to find the intersection

//--------------------------------------------------------------------------------------------------


int ray_aabb_intersect(Ray ray, AABB box) {
    float tx1 = (box.min.x - ray.origin.x) / ray.direction.x;
    float tx2 = (box.max.x - ray.origin.x) / ray.direction.x;
    float tmin = fmin(tx1, tx2);
    float tmax = fmax(tx1, tx2);

    float ty1 = (box.min.y - ray.origin.y) / ray.direction.y;
    float ty2 = (box.max.y - ray.origin.y) / ray.direction.y;
    tmin = fmax(tmin, fmin(ty1, ty2));
    tmax = fmin(tmax, fmax(ty1, ty2));

    float tz1 = (box.min.z - ray.origin.z) / ray.direction.z;
    float tz2 = (box.max.z - ray.origin.z) / ray.direction.z;
    tmin = fmax(tmin, fmin(tz1, tz2));
    tmax = fmin(tmax, fmax(tz1, tz2));

    return tmax >= tmin && tmax > 0;
}

//--------------------------------------------------------------------------------------------------

// ray_bvh_intersect() - Returns the hitrecord for the given ray
// Main function for intersection test by traversing Bounding Volume Hierarchies (BVH) using DFS

//--------------------------------------------------------------------------------------------------


HitRecord ray_bvh_intersect(Ray ray, BVHNode* node) {
    HitRecord rec = {0};
    
    if (!ray_aabb_intersect(ray, node->bounds)) {
        return rec;
    }
    
    if (node->sphere != NULL) {
        return ray_sphere_intersect(ray, node->sphere);
    }
    
    HitRecord left_hit = ray_bvh_intersect(ray, node->left);
    HitRecord right_hit = ray_bvh_intersect(ray, node->right);
    
    if (!left_hit.hit_something) return right_hit;
    if (!right_hit.hit_something) return left_hit;
    
    return (left_hit.t < right_hit.t) ? left_hit : right_hit;
}