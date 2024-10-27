#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIDTH 800
#define HEIGHT 600
#define NUM_SPHERES 10
#define MOVE_SPEED 0.1f
#define ROTATE_SPEED 0.005f

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
Sphere create_random_sphere(int is_glass) {
    Sphere sphere = {
        .center = {random_float(-5, 5), random_float(0, 5), random_float(-5, 5)},
        .radius = random_float(0.5f, 1.5f),
        .color = {rand() % 256, rand() % 256, rand() % 256, 255},
        .reflectivity = is_glass ? 0.9f : random_float(0.0f, 0.5f),
        .transparency = is_glass ? 0.9f : random_float(0.0f, 0.5f),
        .refractive_index = is_glass ? 1.5f : 1.0f,
        .diffuse = random_float(0.1f, 0.9f),
        .specular = random_float(1.0f, 32.0f)
    };

    return sphere;
}

// Function to trace rays and determine color
SDL_Color trace_ray(Vec3 origin, Vec3 direction, Sphere *spheres, int num_spheres, Ground ground, Vec3 light_pos, int depth) {
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
        Vec3 light_dir_to_hit = vec3_subtract(light_pos, hit_point);
        light_dir_to_hit = vec3_normalize(light_dir_to_hit);
        float light_intensity = fmax(0, vec3_dot(normal, light_dir_to_hit));
        color.r = fmin(color.r * light_intensity, 255);
        color.g = fmin(color.g * light_intensity, 255);
        color.b = fmin(color.b * light_intensity, 255);
    }

    return color;
}

// Function to render the scene
void render(SDL_Renderer *renderer, Sphere *spheres, int num_spheres, Ground ground, Vec3 light_pos, int width, int height, Vec3 camera_position) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Normalize device coordinates to [-1, 1]
            float nx = (2.0f * x) / width - 1.0f;
            float ny = 1.0f - (2.0f * y) / height;
            Vec3 ray_direction = vec3_normalize((Vec3){nx, ny, -1}); // Forward direction

            SDL_Color color = trace_ray(camera_position, ray_direction, spheres, num_spheres, ground, light_pos, 0);
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
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
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Ray Tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    // Create random spheres and add glassy ones
    Sphere spheres[NUM_SPHERES];
    for (int i = 0; i < NUM_SPHERES; i++) {
        if (i < NUM_SPHERES / 3) {
            spheres[i] = create_random_sphere(1); // Create glassy spheres
        } else {
            spheres[i] = create_random_sphere(0); // Create regular spheres
        }
    }

    // Create ground
    Ground ground = {
        .y = 0,
        .color = {100, 100, 100, 255},  // Gray ground
        .reflectivity = 0.2f
    };

    // Light source position (huge yellow light sphere)
    Vec3 light_pos = {0, 5, -5}; // Positioned above and behind the camera
    Vec3 light_size = {5.0f, 5.0f, 5.0f}; // Scale for visual representation

    // Camera position and rotation
    Vec3 camera_position = {0, 2, 5};
    float camera_yaw = 0;
    float camera_pitch = 0;
    int dragging = 0;

    SDL_SetRelativeMouseMode(SDL_TRUE); // Enable relative mouse mode

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            // Camera movement
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_w: camera_position.z -= MOVE_SPEED * cos(camera_yaw); camera_position.x -= MOVE_SPEED * sin(camera_yaw); break;  // Move forward
                    case SDLK_s: camera_position.z += MOVE_SPEED * cos(camera_yaw); camera_position.x += MOVE_SPEED * sin(camera_yaw); break;  // Move backward
                    case SDLK_a: camera_position.x -= MOVE_SPEED * cos(camera_yaw); camera_position.z += MOVE_SPEED * sin(camera_yaw); break;  // Move left
                    case SDLK_d: camera_position.x += MOVE_SPEED * cos(camera_yaw); camera_position.z -= MOVE_SPEED * sin(camera_yaw); break;  // Move right
                    case SDLK_SPACE: camera_position.y += MOVE_SPEED; break; // Move up
                    case SDLK_LSHIFT: camera_position.y -= MOVE_SPEED; break; // Move down
                }
            }

            // Mouse rotation
            if (event.type == SDL_MOUSEMOTION && dragging) {
                camera_yaw -= (event.motion.xrel * ROTATE_SPEED);
                camera_pitch -= (event.motion.yrel * ROTATE_SPEED);
                camera_pitch = fmax(fmin(camera_pitch, M_PI / 2 - 0.1), -M_PI / 2 + 0.1); // Limit pitch
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                dragging = 1;
            }
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                dragging = 0;
            }
        }

        // Render scene
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Clear to black
        SDL_RenderClear(renderer);
        render(renderer, spheres, NUM_SPHERES, ground, light_pos, WIDTH, HEIGHT, camera_position);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
