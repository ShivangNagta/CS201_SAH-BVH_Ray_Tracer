#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "Custom/constants.h"
#include "Custom/camera.h"
#include "Custom/sphere.h"
#include "Custom/bvh.h"
#include "Custom/ray.h"
#include "Custom/renderer.h"
#include "Custom/hit.h"
#include "Custom/vec3.h"




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
        .position = {2, 4, 5},
        .forward = {0, 0, -1},
        .right = {1, 0, 0},
        .up = {0, 1, 0},
        .yaw = -M_PI,
        .pitch = 0
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
                        camera.position = vec3_sub(camera.position, 
                            vec3_multiply(camera.forward, MOVE_SPEED));
                        break;
                    case SDLK_a:
                        camera.position = vec3_sub(camera.position, 
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
                SDL_Color color = trace_ray(ray, spheres, NUM_SPHERES, MAX_DEPTH, 
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