#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define MAX_DEPTH 5
#define NUM_SPHERES 5

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
} Sphere;

// Ground Plane
typedef struct {
    float y;  // height of the ground
    SDL_Color color;
    float reflectivity;
} Ground;

Vec3 vec3_add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 vec3_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 vec3_mul(Vec3 a, float scalar) { return (Vec3){a.x * scalar, a.y * scalar, a.z * scalar}; }
Vec3 vec3_normalize(Vec3 v) {
    float mag = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return (Vec3){v.x / mag, v.y / mag, v.z / mag};
}
float vec3_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 vec3_reflect(Vec3 v, Vec3 normal) {
    return vec3_sub(v, vec3_mul(normal, 2 * vec3_dot(v, normal)));
}

float random_float(float min, float max) {
    return min + (float)rand() / (float)(RAND_MAX / (max - min));
}

int ray_sphere_intersect(Ray ray, Sphere sphere, float *t) {
    Vec3 oc = vec3_sub(ray.origin, sphere.center);
    float a = vec3_dot(ray.direction, ray.direction);
    float b = 2.0f * vec3_dot(oc, ray.direction);
    float c = vec3_dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) return 0;
    *t = (-b - sqrt(discriminant)) / (2.0f * a);
    return *t >= 0;
}

// Function to check ray-plane intersection
int ray_ground_intersect(Ray ray, Ground ground, float *t) {
    if (fabs(ray.direction.y) < 1e-6) return 0; // Parallel to the ground
    *t = (ground.y - ray.origin.y) / ray.direction.y;
    return (*t >= 0);
}

SDL_Color blend_color(SDL_Color color1, SDL_Color color2, float blend) {
    return (SDL_Color){
        .r = (int)(color1.r * (1 - blend) + color2.r * blend),
        .g = (int)(color1.g * (1 - blend) + color2.g * blend),
        .b = (int)(color1.b * (1 - blend) + color2.b * blend),
        .a = 255
    };
}

int is_in_shadow(Vec3 point, Sphere *spheres, int num_spheres, Vec3 light_dir) {
    Ray shadow_ray = {point, light_dir};
    for (int i = 0; i < num_spheres; i++) {
        float t;
        if (ray_sphere_intersect(shadow_ray, spheres[i], &t) && t > 0.001f) {
            return 1;
        }
    }
    return 0;
}

SDL_Color trace_ray(Ray ray, Sphere *spheres, int num_spheres, Ground ground, Vec3 light_dir, int depth) {
    float closest_t = INFINITY;
    SDL_Color color = {135, 206, 235, 255};  // Sky color by default
    int hit_sphere = -1;
    float t;
    Vec3 hit_point, hit_normal;

    // Check sphere intersections
    for (int i = 0; i < num_spheres; i++) {
        if (ray_sphere_intersect(ray, spheres[i], &t) && t < closest_t) {
            closest_t = t;
            hit_sphere = i;
            hit_point = vec3_add(ray.origin, vec3_mul(ray.direction, t));
            hit_normal = vec3_normalize(vec3_sub(hit_point, spheres[i].center));
            color = spheres[i].color;
        }
    }

    // Check ground intersection
    if (ray_ground_intersect(ray, ground, &t) && t < closest_t) {
        closest_t = t;
        hit_point = vec3_add(ray.origin, vec3_mul(ray.direction, t));
        hit_normal = (Vec3){0, 1, 0};  // Normal pointing up
        color = ground.color;
        hit_sphere = -1;  // Indicate ground hit
    }

    if (hit_sphere != -1 || closest_t < INFINITY) {
        float intensity = fmax(0, vec3_dot(hit_normal, light_dir)) + 0.2f;  // Diffuse + ambient

        // Shadow
        if (is_in_shadow(vec3_add(hit_point, vec3_mul(hit_normal, 0.001f)), spheres, num_spheres, light_dir)) {
            intensity *= 0.3f;
        }

        color.r *= intensity;
        color.g *= intensity;
        color.b *= intensity;

        // Reflection
        if (depth > 0 && hit_sphere != -1 && spheres[hit_sphere].reflectivity > 0) {
            Vec3 reflect_dir = vec3_reflect(ray.direction, hit_normal);
            Ray reflect_ray = {vec3_add(hit_point, vec3_mul(hit_normal, 0.001f)), reflect_dir};
            SDL_Color reflect_color = trace_ray(reflect_ray, spheres, num_spheres, ground, light_dir, depth - 1);
            color = blend_color(color, reflect_color, spheres[hit_sphere].reflectivity);
        }

        // Transparency
        if (depth > 0 && hit_sphere != -1 && spheres[hit_sphere].transparency > 0) {
            Ray refract_ray = {vec3_add(hit_point, vec3_mul(hit_normal, 0.001f)), ray.direction};
            SDL_Color refract_color = trace_ray(refract_ray, spheres, num_spheres, ground, light_dir, depth - 1);
            color = blend_color(color, refract_color, spheres[hit_sphere].transparency);
        }
    }

    return color;
}

void render(SDL_Renderer *renderer, Sphere *spheres, int num_spheres, Ground ground, Vec3 light_dir, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float u = (2.0f * x / width - 1.0f);
            float v = (2.0f * y / height - 1.0f);
            Ray ray = {{0, 1, 0}, {u, v, -1}};
            ray.direction = vec3_normalize(ray.direction);

            SDL_Color color = trace_ray(ray, spheres, num_spheres, ground, light_dir, MAX_DEPTH);

            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

#ifdef __APPLE__
int main(int argc, char* argv[])
#else
int SDL_main(int argc, char* argv[])
#endif
{
    srand(time(NULL));  // Seed for random sphere generation

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Ray Tracer with Ground and More Features", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create random spheres with different colors and properties
    Sphere spheres[NUM_SPHERES];
    for (int i = 0; i < NUM_SPHERES; i++) {
        spheres[i] = (Sphere){
            .center = {random_float(-5, 5), random_float(0.5f, 2.0f), random_float(-5, 5)},
            .radius = random_float(0.5f, 1.5f),
            .color = {rand() % 256, rand() % 256, rand() % 256, 255},
            .reflectivity = random_float(0.0f, 0.5f),
            .transparency = random_float(0.0f, 0.5f),
            .refractive_index = 1.5f
        };
    }

    // Create ground
    Ground ground = {
        .y = 0,
        .color = {100, 100, 100, 255},  // Gray ground
        .reflectivity = 0.2f
    };

    Vec3 light_dir = {0.5f, -1.0f, -0.5f};  // Direction of light source
    light_dir = vec3_normalize(light_dir);

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        render(renderer, spheres, NUM_SPHERES, ground, light_dir, 800, 600);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


