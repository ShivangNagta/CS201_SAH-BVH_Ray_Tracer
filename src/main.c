#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600
#define NUM_SPHERES 500
#define MOVE_SPEED 0.5f
#define ROTATE_SPEED 0.002f
#define MAX_DEPTH 5
#define EPSILON 0.0001f

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    Vec3 origin;
    Vec3 direction;
} Ray;

typedef struct {
    Vec3 center;
    float radius;
    SDL_Color color;
    float reflectivity;
    float transparency;
    float refractive_index;
    float diffuse;
    float specular;
    int is_light;  // New flag for light sources
} Sphere;

typedef struct {
    float y;
    SDL_Color color;
    float reflectivity;
} Ground;

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


// Vector operations
Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vec3_subtract(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 vec3_multiply(Vec3 v, float t) {
    return (Vec3){v.x * t, v.y * t, v.z * t};
}

Vec3 vec3_normalize(Vec3 v) {
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return (Vec3){v.x / length, v.y / length, v.z / length};
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 vec3_reflect(Vec3 v, Vec3 n) {
    float dot = vec3_dot(v, n);
    return vec3_subtract(v, vec3_multiply(n, 2 * dot));
}

Vec3 vec3_refract(Vec3 uv, Vec3 n, float etai_over_etat) {
    float cos_theta = fmin(vec3_dot(vec3_multiply(uv, -1), n), 1.0);
    Vec3 r_out_perp = vec3_multiply(
        vec3_add(uv, vec3_multiply(n, cos_theta)),
        etai_over_etat
    );
    Vec3 r_out_parallel = vec3_multiply(
        n,
        -sqrt(fabs(1.0 - vec3_dot(r_out_perp, r_out_perp)))
    );
    return vec3_add(r_out_perp, r_out_parallel);
}

// Camera structure and functions
typedef struct {
    Vec3 position;
    Vec3 forward;
    Vec3 right;
    Vec3 up;
    float yaw;
    float pitch;
} Camera;

void camera_update(Camera *camera) {
    // Update forward vector based on yaw and pitch
    camera->forward.x = cos(camera->pitch) * sin(camera->yaw);
    camera->forward.y = sin(camera->pitch);
    camera->forward.z = cos(camera->pitch) * cos(camera->yaw);
    camera->forward = vec3_normalize(camera->forward);
    
    // Update right vector
    camera->right = vec3_normalize(vec3_cross(camera->forward, (Vec3){0, 1, 0}));
    
    // Update up vector
    camera->up = vec3_normalize(vec3_cross(camera->right, camera->forward));
}

Ray get_camera_ray(Camera *camera, float u, float v) {
    Vec3 horizontal = vec3_multiply(camera->right, 2.0f);
    Vec3 vertical = vec3_multiply(camera->up, 2.0f);
    
    Vec3 direction = camera->forward;
    direction = vec3_add(direction, vec3_multiply(horizontal, u));
    direction = vec3_add(direction, vec3_multiply(vertical, v));
    direction = vec3_normalize(direction);
    
    return (Ray){camera->position, direction};
}

// Random functions
float random_float(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

Vec3 random_in_unit_sphere() {
    while (1) {
        Vec3 p = {
            random_float(-1, 1),
            random_float(-1, 1),
            random_float(-1, 1)
        };
        if (vec3_dot(p, p) < 1) return p;
    }
}

// Create objects
Sphere create_random_sphere(int is_glass) {
    Sphere sphere = {
        .center = {random_float(-10, 5), random_float(-5, 6), random_float(-10, 5)},
        .radius = random_float(0.5f, 1.5f),
        .color = {rand() % 256, rand() % 256, rand() % 256, 255},
        .reflectivity = is_glass ? 0.9f : random_float(0.0f, 0.5f),
        .transparency = is_glass ? 0.9f : 0.0f,
        .refractive_index = is_glass ? 1.5f : 1.0f,
        .diffuse = is_glass ? 0.1f : random_float(0.1f, 0.9f),
        .specular = is_glass ? 32.0f : random_float(1.0f, 32.0f),
        .is_light = 0
    };
    return sphere;
}

Sphere create_light_sphere() {
    Sphere light = {
        .center = {15, 4, -2},
        .radius = 10.0f,
        .color = {255, 255, 200, 255},  // Warm light color
        .reflectivity = 0.0f,
        .transparency = 0.0f,
        .refractive_index = 1.0f,
        .diffuse = 0.0f,
        .specular = 0.0f,
        .is_light = 1
    };
    return light;
}

// Ray intersection functions
typedef struct {
    float t;
    Vec3 point;
    Vec3 normal;
    int hit_something;
    Sphere *object;
} HitRecord;

HitRecord ray_sphere_intersect(Ray ray, Sphere *sphere) {
    HitRecord rec = {0};
    Vec3 oc = vec3_subtract(ray.origin, sphere->center);
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
            rec.normal = vec3_normalize(vec3_subtract(rec.point, sphere->center));
            rec.object = sphere;
            return rec;
        }
    }
    return rec;
}




// AABB operations
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

// Modified ray tracing function to use BVH
HitRecord ray_bvh_intersect(Ray ray, BVHNode* node) {
    HitRecord rec = {0};
    
    if (!ray_aabb_intersect(ray, node->bounds)) {
        return rec;
    }
    
    if (node->sphere != NULL) {
        // Leaf node
        return ray_sphere_intersect(ray, node->sphere);
    }
    
    // Internal node - traverse children
    HitRecord left_hit = ray_bvh_intersect(ray, node->left);
    HitRecord right_hit = ray_bvh_intersect(ray, node->right);
    
    if (!left_hit.hit_something) return right_hit;
    if (!right_hit.hit_something) return left_hit;
    
    return (left_hit.t < right_hit.t) ? left_hit : right_hit;
}





SDL_Color trace_ray(Ray ray, Sphere *spheres, int num_spheres, Ground ground, int depth, BVHNode* bvh) {
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
            closest_hit.object->color : ground.color;

        // Calculate lighting contribution from all light sources
        for (int i = 0; i < num_spheres; i++) {
            if (spheres[i].is_light) {
                // Calculate light direction and distance
                Vec3 light_dir = vec3_subtract(spheres[i].center, closest_hit.point);
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
                SDL_Color reflect_color = trace_ray(reflect_ray, spheres, num_spheres, ground, depth - 1, bvh);
                
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
                SDL_Color refract_color = trace_ray(refract_ray, spheres, num_spheres, ground, depth - 1, bvh);
                
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


// Main function and initialization code for the raytracer
int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Raytracer", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize random seed
    srand(time(NULL));

    // Create camera
    Camera camera = {
        .position = {2, 4, 15},
        .forward = {0, 0, -1},
        .right = {1, 0, 0},
        .up = {0, 1, 0},
        .yaw = -M_PI,
        .pitch = 0
    };

    // Create ground
    Ground ground = {
        .y = -0.5f,
        .color = {100, 100, 100, 255},
        .reflectivity = 0.1f
    };

    // Create spheres
    Sphere spheres[NUM_SPHERES];
    
    // Create one light source
    spheres[0] = create_light_sphere();
    
    // Create one glass sphere
    spheres[1] = create_random_sphere(1);  // Glass sphere
    spheres[1].center = (Vec3){0, 1, 0};   // Place it in a specific position
    
    // Create remaining random spheres
    for (int i = 2; i < NUM_SPHERES; i++) {
        spheres[i] = create_random_sphere(0);
    }

    printf("Building BVH...\n");
    struct timespec bvh_start, bvh_end;
    clock_gettime(CLOCK_MONOTONIC, &bvh_start);
    BVHNode* root = build_bvh_node(spheres, 0, NUM_SPHERES, 0);
    clock_gettime(CLOCK_MONOTONIC, &bvh_end);
    double bvh_build_time = (bvh_end.tv_sec - bvh_start.tv_sec) + 
                           (bvh_end.tv_nsec - bvh_start.tv_nsec) / 1e9;
    printf("BVH built in %f seconds\n", bvh_build_time);

    // Main loop flag
    int quit = 0;
    SDL_Event e;


        // Add performance tracking variables
    int frame_count = 0;
    struct timespec frame_start, frame_end;
    double total_render_time = 0;

    // Toggle for BVH
    int use_bvh = 1;  // Start with BVH enabled

    // Game loop
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_w:
                        camera.position = vec3_add(camera.position, 
                            vec3_multiply(camera.forward, MOVE_SPEED));
                        break;
                    case SDLK_s:
                        camera.position = vec3_subtract(camera.position, 
                            vec3_multiply(camera.forward, MOVE_SPEED));
                        break;
                    case SDLK_a:
                        camera.position = vec3_subtract(camera.position, 
                            vec3_multiply(camera.right, MOVE_SPEED));
                        break;
                    case SDLK_d:
                        camera.position = vec3_add(camera.position, 
                            vec3_multiply(camera.right, MOVE_SPEED));
                        break;
                    case SDLK_SPACE:
                        camera.position.y += MOVE_SPEED;
                        break;
                    case SDLK_LSHIFT:
                        camera.position.y -= MOVE_SPEED;
                        break;
                    case SDLK_b:  // Toggle BVH
                        use_bvh = !use_bvh;
                        printf("BVH %s\n", use_bvh ? "enabled" : "disabled");
                        break;
                }
            }
            else if (e.type == SDL_MOUSEMOTION) {
                if (e.motion.state & SDL_BUTTON_LMASK) {
                    camera.yaw += e.motion.xrel * ROTATE_SPEED;
                    camera.pitch -= e.motion.yrel * ROTATE_SPEED;
                    camera.pitch = fmax(fmin(camera.pitch, M_PI/2 - 0.1f), -M_PI/2 + 0.1f);
                    camera_update(&camera);
                }
            }
        }

                // Start frame timing
        clock_gettime(CLOCK_MONOTONIC, &frame_start);

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render scene
        #pragma omp parallel for collapse(2)
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                float u = (float)x / WIDTH - 0.5f;
                float v = (float)y / HEIGHT - 0.5f;
                
                Ray ray = get_camera_ray(&camera, u, -v);
                SDL_Color color = trace_ray(ray, spheres, NUM_SPHERES, ground, MAX_DEPTH, 
                                          use_bvh ? root : NULL);
                
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        // Update screen
        SDL_RenderPresent(renderer);

                // End frame timing
        clock_gettime(CLOCK_MONOTONIC, &frame_end);
        double frame_time = (frame_end.tv_sec - frame_start.tv_sec) + 
                          (frame_end.tv_nsec - frame_start.tv_nsec) / 1e9;
        total_render_time += frame_time;
        frame_count++;

        // Print performance stats every 10 frames
        if (frame_count % 10 == 0) {
            printf("Average frame time: %f seconds (%.2f FPS)\n", 
                   total_render_time / frame_count,
                   frame_count / total_render_time);
        }
    }

    // Final performance report
    printf("\nFinal Performance Report:\n");
    printf("Total frames: %d\n", frame_count);
    printf("Average frame time: %f seconds\n", total_render_time / frame_count);
    printf("Average FPS: %.2f\n", frame_count / total_render_time);
    printf("BVH build time: %f seconds\n", bvh_build_time);

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}