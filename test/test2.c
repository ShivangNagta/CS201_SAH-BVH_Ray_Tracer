#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

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
} Sphere;

typedef struct {
    Vec3 point;
    Vec3 normal;
    SDL_Color color;
    float reflectivity;
} Plane;

// Vector operations
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

int ray_plane_intersect(Ray ray, Plane plane, float *t) {
    float denom = vec3_dot(plane.normal, ray.direction);
    if (fabs(denom) > 1e-6) {
        Vec3 p0l0 = vec3_sub(plane.point, ray.origin);
        *t = vec3_dot(p0l0, plane.normal) / denom;
        return *t >= 0;
    }
    return 0;
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

SDL_Color trace_ray(Ray ray, Sphere *spheres, int num_spheres, Plane ground, Vec3 light_dir, int depth) {
    float closest_t = INFINITY;
    SDL_Color color = {135, 206, 235, 255};  // Default sky color

    int hit_sphere = -1;
    int hit_plane = 0;
    Vec3 hit_point, hit_normal;
    float t;

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
    if (ray_plane_intersect(ray, ground, &t) && t < closest_t) {
        closest_t = t;
        hit_plane = 1;
        hit_point = vec3_add(ray.origin, vec3_mul(ray.direction, t));
        hit_normal = ground.normal;
        color = ground.color;
    }

    if (hit_sphere != -1 || hit_plane) {
        float intensity = fmax(0, vec3_dot(hit_normal, light_dir));

        if (is_in_shadow(vec3_add(hit_point, vec3_mul(hit_normal, 0.001f)), spheres, num_spheres, light_dir)) {
            intensity *= 0.2f;  // Darken if in shadow
        }

        color.r *= intensity;
        color.g *= intensity;
        color.b *= intensity;

        // Reflection
        if (depth > 0 && ((hit_sphere != -1 && spheres[hit_sphere].reflectivity > 0) || hit_plane)) {
            Vec3 reflect_dir = vec3_reflect(ray.direction, hit_normal);
            Ray reflect_ray = {vec3_add(hit_point, vec3_mul(hit_normal, 0.001f)), reflect_dir};
            SDL_Color reflect_color = trace_ray(reflect_ray, spheres, num_spheres, ground, light_dir, depth - 1);

            color.r = (int)(color.r * (1 - spheres[hit_sphere].reflectivity) + reflect_color.r * spheres[hit_sphere].reflectivity);
            color.g = (int)(color.g * (1 - spheres[hit_sphere].reflectivity) + reflect_color.g * spheres[hit_sphere].reflectivity);
            color.b = (int)(color.b * (1 - spheres[hit_sphere].reflectivity) + reflect_color.b * spheres[hit_sphere].reflectivity);
        }
    }

    return color;
}

void render(SDL_Renderer *renderer, Sphere *spheres, int num_spheres, Plane ground, Vec3 light_dir, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float u = (2.0f * x / width - 1.0f);
            float v = (2.0f * y / height - 1.0f);
            Ray ray = {{0, 1, 0}, {u, v, -1}};
            ray.direction = vec3_normalize(ray.direction);

            SDL_Color color = trace_ray(ray, spheres, num_spheres, ground, light_dir, 3);

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
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Enhanced Raytracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
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

    Sphere spheres[] = {
        {{0, 1, -5}, 1, {255, 0, 0, 255}, 0.5f},   // Red reflective sphere
        {{2, 1, -7}, 1, {0, 255, 0, 255}, 0.3f},   // Green reflective sphere
        {{-2, 1, -6}, 1, {0, 0, 255, 255}, 0.7f}   // Blue reflective sphere
    };
    int num_spheres = sizeof(spheres) / sizeof(Sphere);

    Plane ground = {{0, 0, 0}, {0, 1, 0}, {100, 100, 100, 255}, 0.5f};  // Gray ground

    Vec3 light_dir = vec3_normalize((Vec3){-1, -1, -1});

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_RenderClear(renderer);
        render(renderer, spheres, num_spheres, ground, light_dir, 800, 600);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
