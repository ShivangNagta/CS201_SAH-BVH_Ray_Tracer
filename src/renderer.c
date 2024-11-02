#include "Custom/renderer.h"
#include "Custom/hit.h"
#include <math.h>

//--------------------------------------------------------------------------------------------------

// Main function for recursive ray tracing (up to a specified depth).
// The closest intersection of the ray can be calculated using two methods:
// Method 1 - Bounding Volume Hierarchies (BVH) with Surface Area Heuristics (SAH) - O(log n), but building the BVH is O(n log n).
// Method 2 - Brute force, traversing all objects (spheres) - O(n) for intersection tests.

// Recursion ends earlier if the ray directly misses all objects or ends up going towards
// the sky after reflection.
// This is a naive implementation of the microfacet model, considering only material roughness.
// Many other material properties are not included for simplicity.
// A given ray takes the base color of the material it hits and blends it with the color
// of the reflected ray recursively.

//--------------------------------------------------------------------------------------------------

SDL_Color trace_ray(Ray ray, Sphere *spheres, int num_spheres, int depth, BVHNode *bvh)
{
    if (depth <= 0)
        return (SDL_Color){0, 0, 0, 255};

    HitRecord closest_hit = {0};
    closest_hit.t = INFINITY;

    if (bvh)
    {
        closest_hit = ray_bvh_intersect(ray, bvh);
    }
    else
    {

        for (int i = 0; i < num_spheres; i++)
        {
            HitRecord hit = ray_sphere_intersect(ray, &spheres[i]);
            if (hit.hit_something && hit.t < closest_hit.t)
            {
                closest_hit = hit;
            }
        }
    }

    if (closest_hit.hit_something)
    {
        SDL_Color final_color = {0, 0, 0, 255};
        SDL_Color base_color = closest_hit.object ? closest_hit.object->color : (SDL_Color){0.0f, 0.0f, 0.0f};

        Vec3 reflected_dir = random_on_hemisphere(closest_hit.normal);
        // Vec3 reflected_dir = vec3_reflect(closest_hit.point, closest_hit.normal);

        Ray reflected_ray = {closest_hit.point, reflected_dir};
        SDL_Color reflected_color = trace_ray(reflected_ray, spheres, num_spheres, depth - 1, bvh);
        final_color.r = (Uint8)(base_color.r + 0.5 * reflected_color.r);
        final_color.g = (Uint8)(base_color.g + 0.5 * reflected_color.g);
        final_color.b = (Uint8)(base_color.b + 0.5 * reflected_color.b);

        final_color.a = 255;

        return final_color;
    }

    float t = 0.5f * (ray.direction.y + 1.0f);
    SDL_Color sky_color = {
        (1.0f - t) * 255 + t * 128,
        (1.0f - t) * 255 + t * 178,
        255,
        255};
    // SDL_Color sky_color = {
    //     255,
    //     255,
    //     255,
    //     255};
    return sky_color;
}
