#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600
#define NUM_SPHERES 10
#define MOVE_SPEED 0.1f
#define NOISE_AMOUNT 0.1f // Maximum noise displacement
#define NOISE_RECOVERY_RATE 0.05f // Rate at which noise diminishes

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    Vec3 center;
    float radius;
    SDL_Color color;
    float reflectivity;
    float transparency;
    float refractive_index;
    float diffuse;
    float specular;
} Sphere;

typedef struct {
    float y;
    SDL_Color color;
    float reflectivity;
} Ground;

// Function to generate random float between min and max
float random_float(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

// Function to normalize a vector
Vec3 vec3_normalize(Vec3 v) {
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return (Vec3){v.x / length, v.y / length, v.z / length};
}

// Function to compute dot product of two vectors
float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Function to subtract two vectors
Vec3 vec3_subtract(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

// Function to create a random sphere
Sphere create_random_sphere() {
    return (Sphere){
        .center = {random_float(-5, 5), random_float(0, 5), random_float(-5, 5)},
        .radius = random_float(0.5f, 1.5f),
        .color = {rand() % 256, rand() % 256, rand() % 256, 255},
        .reflectivity = random_float(0.0f, 0.5f),
        .transparency = random_float(0.0f, 0.5f),
        .refractive_index = 1.5f,
        .diffuse = random_float(0.1f, 0.9f),
        .specular = random_float(1.0f, 32.0f)
    };
}

// Function to trace rays and determine color
SDL_Color trace_ray(Vec3 origin, Vec3 direction, Sphere *spheres, int num_spheres, Ground ground, Vec3 light_dir, int depth) {
    SDL_Color color = {0, 0, 0, 255}; // Background color (black)
    float nearest_t = INFINITY;
    Sphere *nearest_sphere = NULL;

    // Check for intersection with spheres
    for (int i = 0; i < num_spheres; i++) {
        Sphere *sphere = &spheres[i];
        Vec3 oc = vec3_subtract(origin, sphere->center);
        float a = vec3_dot(direction, direction);
        float b = 2.0f * vec3_dot(oc, direction);
        float c = vec3_dot(oc, oc) - sphere->radius * sphere->radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant > 0) {
            float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
            float t2 = (-b + sqrt(discriminant)) / (2.0f * a);
            float t = fminf(t1, t2);

            if (t < nearest_t && t > 0) {
                nearest_t = t;
                nearest_sphere = sphere;
            }
        }
    }

    // Check for intersection with ground
    if (origin.y > ground.y) {
        float t = (ground.y - origin.y) / direction.y; // t for ground intersection
        if (t > 0 && t < nearest_t) {
            nearest_t = t;
            nearest_sphere = NULL; // Ground is the nearest intersection
        }
    }

    if (nearest_sphere) {
        // Calculate intersection point and normal
        Vec3 hit_point = {origin.x + direction.x * nearest_t, 
                          origin.y + direction.y * nearest_t, 
                          origin.z + direction.z * nearest_t};

        Vec3 normal;
        if (nearest_sphere) {
            normal = vec3_normalize(vec3_subtract(hit_point, nearest_sphere->center));
            color = nearest_sphere->color; // Base color from sphere
        } else {
            normal = (Vec3){0, 1, 0}; // Normal for the ground
            color = ground.color; // Base color from ground
        }

        // Calculate diffuse lighting
        float light_intensity = fmax(0, vec3_dot(normal, light_dir));
        color.r = fmin(color.r * light_intensity, 255);
        color.g = fmin(color.g * light_intensity, 255);
        color.b = fmin(color.b * light_intensity, 255);
    }

    return color;
}

// Function to render the scene
void render(SDL_Renderer *renderer, Sphere *spheres, int num_spheres, Ground ground, Vec3 light_dir, int width, int height, Vec3 camera_position) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Normalize device coordinates to [-1, 1]
            float nx = (2.0f * x) / width - 1.0f;
            float ny = 1.0f - (2.0f * y) / height;
            Vec3 ray_direction = vec3_normalize((Vec3){nx, ny, -1}); // Forward direction

            SDL_Color color = trace_ray(camera_position, ray_direction, spheres, num_spheres, ground, light_dir, 0);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

// Function to generate random float between -1 and 1 for noise
float random_float_noise() {
    return ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
}

#ifdef __APPLE__
int main(int argc, char* argv[])
#else
int SDL_main(int argc, char* argv[])
#endif
{
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Ray Tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    // Create random spheres
    Sphere spheres[NUM_SPHERES];
    for (int i = 0; i < NUM_SPHERES; i++) {
        spheres[i] = create_random_sphere();
    }

    // Create ground
    Ground ground = {
        .y = 0,
        .color = {100, 100, 100, 255},  // Gray ground
        .reflectivity = 0.2f
    };

    Vec3 light_dir = {0.5f, -1.0f, -0.5f};
    light_dir = vec3_normalize(light_dir);

    // Camera position
    Vec3 camera_position = {0, 2, 5};

    // Noise variables
    Vec3 noise_offset = {0, 0, 0}; // Current noise offset
    int moving = 0; // Track if the camera is moving

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            // Camera movement
            if (event.type == SDL_KEYDOWN) {
                moving = 1; // Set moving flag
                switch (event.key.keysym.sym) {
                    case SDLK_w: camera_position.z -= MOVE_SPEED; break;  // Move forward
                    case SDLK_s: camera_position.z += MOVE_SPEED; break;  // Move backward
                    case SDLK_a: camera_position.x -= MOVE_SPEED; break;  // Move left
                    case SDLK_d: camera_position.x += MOVE_SPEED; break;  // Move right
                    case SDLK_SPACE: camera_position.y += MOVE_SPEED; break; // Move up
                    case SDLK_LSHIFT: camera_position.y -= MOVE_SPEED; break; // Move down
                }
            }
        }

        // Apply noise if moving
        if (moving) {
            noise_offset.x = random_float_noise() * NOISE_AMOUNT;
            noise_offset.y = random_float_noise() * NOISE_AMOUNT;
            noise_offset.z = random_float_noise() * NOISE_AMOUNT;
        } else {
            // Gradually recover noise
            noise_offset.x *= (1.0f - NOISE_RECOVERY_RATE);
            noise_offset.y *= (1.0f - NOISE_RECOVERY_RATE);
            noise_offset.z *= (1.0f - NOISE_RECOVERY_RATE);
        }

        // Update camera position with noise
        Vec3 noisy_camera_position = {
            camera_position.x + noise_offset.x,
            camera_position.y + noise_offset.y,
            camera_position.z + noise_offset.z
        };

        // Render scene
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        render(renderer, spheres, NUM_SPHERES, ground, light_dir, WIDTH, HEIGHT, noisy_camera_position);
        SDL_RenderPresent(renderer);
        moving = 0; // Reset moving flag
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
