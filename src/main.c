#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL_image.h>
#include "Custom/constants.h"
#include "Custom/camera.h"
#include "Custom/sphere.h"
#include "Custom/bvh.h"
#include "Custom/ray.h"
#include "Custom/renderer.h"
#include "Custom/hit.h"
#include "Custom/vec3.h"
#include "Custom/benchmark.h"





// Modified plot display function
void display_plot_with_sdl(SDL_Renderer* renderer) {
    // Initialize SDL_image with PNG support
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return;
    }

    // Load the generated PNG
    SDL_Surface* plot_surface = IMG_Load("benchmark_results.png");
    if (!plot_surface) {
        printf("Error loading plot: %s\n", IMG_GetError());
        return;
    }
    
    // Create texture from surface
    SDL_Texture* plot_texture = SDL_CreateTextureFromSurface(renderer, plot_surface);
    SDL_FreeSurface(plot_surface);
    
    if (!plot_texture) {
        printf("Error creating texture: %s\n", SDL_GetError());
        return;
    }
    
    // Get texture size
    int width, height;
    SDL_QueryTexture(plot_texture, NULL, NULL, &width, &height);
    
    // Create destination rectangle
    SDL_Rect dest_rect = {
        .x = (WIDTH - width) / 2,   // Center horizontally
        .y = (HEIGHT - height) / 2,  // Center vertically
        .w = width,
        .h = height
    };
    
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render plot
    SDL_RenderCopy(renderer, plot_texture, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
    
    // Cleanup
    SDL_DestroyTexture(plot_texture);
    IMG_Quit();
}

#ifdef __APPLE__
int main(int argc, char* argv[])
#else
int SDL_main(int argc, char* argv[])
#endif
{
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Raytracer", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("\nPlease proceed as follows :\n\n");
    printf("Press '1' for benchmark testing with graph plot.\n");
    printf("Press '2' for Realtime CPU Raytracing.\n");
    printf("Press '3' for Static Rendering comparison with and without optimisation. (TODO) \n");
    printf("Press '4' for visulisation of Bounding Volume Hierarchies on mesh object. (TODO) \n\n");
    printf("Waiting for the input : ");

    int input;
    scanf("%d", &input);

    switch (input)
    {

//------------------------------------------------------------------------------------------

    // Benchmark Testing

    case 1:
    {

        run_benchmark_with_plotting();
        if (renderer) {
            display_plot_with_sdl(renderer);
            // Wait for user input to continue
            SDL_Event e;
            while (SDL_WaitEvent(&e)) {
                if (e.type == SDL_QUIT || 
                    (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                    break;
                }
            }
        }
        break;

    }
//------------------------------------------------------------------------------------------


    //Realtime Raytracing   
    case 2:
        {

        Camera camera = {
            .position = {2, 4, 5},
            .forward = {0, 0, -1},
            .right = {1, 0, 0},
            .up = {0, 1, 0},
            .yaw = -M_PI,
            .pitch = 0
        };


        Sphere spheres[NUM_SPHERES];
        
        spheres[0] = create_light_sphere();   
        spheres[1] = create_random_sphere(1);  
        spheres[1].center = (Vec3){0, 1, 0};  
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


        int quit = 0;
        SDL_Event e;

        int frame_count = 0;
        struct timespec frame_start, frame_end;
        double total_render_time = 0;

    
        int use_bvh = 1;

        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
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
                        case SDLK_b: 
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

            clock_gettime(CLOCK_MONOTONIC, &frame_start);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);


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


            SDL_RenderPresent(renderer);
            clock_gettime(CLOCK_MONOTONIC, &frame_end);
            double frame_time = (frame_end.tv_sec - frame_start.tv_sec) + 
                            (frame_end.tv_nsec - frame_start.tv_nsec) / 1e9;
            total_render_time += frame_time;
            frame_count++;

            if (frame_count % 10 == 0) {
                printf("Average frame time: %f seconds (%.2f FPS)\n", 
                    total_render_time / frame_count,
                    frame_count / total_render_time);
            }
        }

        printf("\nFinal Performance Report:\n");
        printf("Total frames: %d\n", frame_count);
        printf("Average frame time: %f seconds\n", total_render_time / frame_count);
        printf("Average FPS: %.2f\n", frame_count / total_render_time);
        printf("BVH build time: %f seconds\n", bvh_build_time);


        break;
        }
//------------------------------------------------------------------------------------------

    case 3:
        printf("To do");
        break;
    case 4:
        printf("To do");
        break;
    
    default:
        printf("Please press only among the given options");
        break;
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
 
}