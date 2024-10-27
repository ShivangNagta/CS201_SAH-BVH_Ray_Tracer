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
} Sphere;

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 vec3_mul(Vec3 a, float scalar) {
    return (Vec3){a.x * scalar, a.y * scalar, a.z * scalar};
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

int ray_sphere_intersect(Ray ray, Sphere sphere, float *t) {
    Vec3 oc = vec3_sub(ray.origin, sphere.center);
    float a = vec3_dot(ray.direction, ray.direction);
    float b = 2.0f * vec3_dot(oc, ray.direction);
    float c = vec3_dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) return 0;  // No intersection
    *t = (-b - sqrt(discriminant)) / (2.0f * a);  // Closest intersection
    return *t >= 0;
}

void render(SDL_Renderer *renderer, Sphere *spheres, int num_spheres, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float u = (2.0f * x / width - 1.0f);
            float v = (2.0f * y / height - 1.0f);
            Ray ray = {{0, 0, 0}, {u, v, -1}};  // Simple camera setup

            SDL_Color color = {0, 0, 0, 255};  // Default to black background
            float closest_t = INFINITY;
            for (int i = 0; i < num_spheres; i++) {
                float t;
                if (ray_sphere_intersect(ray, spheres[i], &t) && t < closest_t) {
                    closest_t = t;
                    color = spheres[i].color;
                }
            }

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

    SDL_Window *window = SDL_CreateWindow("Simple Raytracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
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
        {{0, 0, -3}, 1, {255, 0, 0, 255}},  // Red sphere
        {{1.5, 0, -4}, 1, {0, 255, 0, 255}},  // Green sphere
        {{-1.5, 0, -4}, 1, {0, 0, 255, 255}}  // Blue sphere
    };
    int num_spheres = sizeof(spheres) / sizeof(Sphere);

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_RenderClear(renderer);
        render(renderer, spheres, num_spheres, 800, 600);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
