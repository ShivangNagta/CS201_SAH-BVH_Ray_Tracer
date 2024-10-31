#include "Custom/renderer.h"
#include "Custom/hit.h"
#include <math.h>


SDL_Color trace_ray(Ray ray, Sphere *spheres, int num_spheres, int depth, BVHNode* bvh) {
    if (depth <= 0) return (SDL_Color){0, 0, 0, 255};

    HitRecord closest_hit = {0};
    closest_hit.t = INFINITY;

    // Check sphere intersections
    if (bvh) {
        closest_hit = ray_bvh_intersect(ray, bvh);
    } else {
        // Original sphere intersection testing
        for (int i = 0; i < num_spheres; i++) {
            HitRecord hit = ray_sphere_intersect(ray, &spheres[i]);
            if (hit.hit_something && hit.t < closest_hit.t) {
                closest_hit = hit;
            }
        }
    }

    // Check ground intersection
    // if (ray.origin.y > ground.y && ray.direction.y < 0) {
    //     float t = (ground.y - ray.origin.y) / ray.direction.y;
    //     if (t > EPSILON && t < closest_hit.t) {
    //         closest_hit.hit_something = 1;
    //         closest_hit.t = t;
    //         closest_hit.point = vec3_add(ray.origin, vec3_multiply(ray.direction, t));
    //         closest_hit.normal = (Vec3){0, 1, 0};
    //         closest_hit.object = NULL;
    //     }
    // }

    if (closest_hit.hit_something) {
        SDL_Color final_color = {0, 0, 0, 255};
        
        // If we hit a light source, return its color
        if (closest_hit.object && closest_hit.object->is_light) {
            return closest_hit.object->color;
        }

        // Get base color
        SDL_Color base_color = closest_hit.object ? 
            closest_hit.object->color : (SDL_Color){0.0f, 0.0f, 0.0f};

        // Calculate lighting contribution from all light sources
        for (int i = 0; i < num_spheres; i++) {
            if (spheres[i].is_light) {
                // Calculate light direction and distance
                Vec3 light_dir = vec3_sub(spheres[i].center, closest_hit.point);
                float light_distance = sqrt(vec3_dot(light_dir, light_dir));
                light_dir = vec3_normalize(light_dir);

                // Shadow ray
                Ray shadow_ray = {closest_hit.point, light_dir};
                int in_shadow = 0;
                for (int j = 0; j < num_spheres; j++) {
                    if (j != i && !spheres[j].is_light) {
                        HitRecord shadow_hit = ray_sphere_intersect(shadow_ray, &spheres[j]);
                        if (shadow_hit.hit_something && shadow_hit.t < light_distance) {
                            in_shadow = 1;
                            break;
                        }
                    }
                }

                if (!in_shadow) {
                    // Diffuse lighting
                    float diff = fmax(0.0f, vec3_dot(closest_hit.normal, light_dir));
                    
                    // Specular lighting
                    Vec3 view_dir = vec3_normalize(vec3_multiply(ray.direction, -1));
                    Vec3 reflect_dir = vec3_reflect(vec3_multiply(light_dir, -1), closest_hit.normal);
                    float spec = pow(fmax(vec3_dot(view_dir, reflect_dir), 0.0f), 
                                   closest_hit.object ? closest_hit.object->specular : 16.0f);

                    // Light attenuation
                    float attenuation = 1.0f / (1.0f + 0.09f * light_distance + 0.032f * light_distance * light_distance);

                    // Add to final color
                    final_color.r += (base_color.r * diff + 255 * spec) * attenuation;
                    final_color.g += (base_color.g * diff + 255 * spec) * attenuation;
                    final_color.b += (base_color.b * diff + 255 * spec) * attenuation;
                }
            }
        }

        // Add ambient light
        final_color.r = fmin(final_color.r + base_color.r * 0.1f, 255);
        final_color.g = fmin(final_color.g + base_color.g * 0.1f, 255);
        final_color.b = fmin(final_color.b + base_color.b * 0.1f, 255);

        // Handle reflections and refractions
        if (closest_hit.object) {
            if (closest_hit.object->reflectivity > 0) {
                Vec3 reflected = vec3_reflect(ray.direction, closest_hit.normal);
                Ray reflect_ray = {closest_hit.point, reflected};
                SDL_Color reflect_color = trace_ray(reflect_ray, spheres, num_spheres, depth - 1, bvh);
                
                final_color.r = (1 - closest_hit.object->reflectivity) * final_color.r + 
                               closest_hit.object->reflectivity * reflect_color.r;
                final_color.g = (1 - closest_hit.object->reflectivity) * final_color.g + 
                               closest_hit.object->reflectivity * reflect_color.g;
                final_color.b = (1 - closest_hit.object->reflectivity) * final_color.b + 
                               closest_hit.object->reflectivity * reflect_color.b;
            }

            if (closest_hit.object->transparency > 0) {
                float refraction_ratio = ray.direction.y > 0 ? 
                    closest_hit.object->refractive_index : 1.0f / closest_hit.object->refractive_index;
                
                Vec3 refracted = vec3_refract(ray.direction, closest_hit.normal, refraction_ratio);
                Ray refract_ray = {closest_hit.point, refracted};
                SDL_Color refract_color = trace_ray(refract_ray, spheres, num_spheres, depth - 1, bvh);
                
                final_color.r = (1 - closest_hit.object->transparency) * final_color.r + 
                               closest_hit.object->transparency * refract_color.r;
                final_color.g = (1 - closest_hit.object->transparency) * final_color.g + 
                               closest_hit.object->transparency * refract_color.g;
                final_color.b = (1 - closest_hit.object->transparency) * final_color.b + 
                               closest_hit.object->transparency * refract_color.b;
            }
        }

        return final_color;
    }

    // Background color (sky)
    float t = 0.5f * (ray.direction.y + 1.0f);
    SDL_Color sky_color = {
        (1.0f - t) * 255 + t * 128,
        (1.0f - t) * 255 + t * 178,
        255,
        255
    };
    // SDL_Color sky_color = {
    //     0,
    //     0,
    //     0,
    //     255
    // };
    return sky_color;
}