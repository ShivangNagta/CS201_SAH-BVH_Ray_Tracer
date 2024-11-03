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
#include "Custom/bvh_visualiser.h"

#define NUM_SPHERES 20
#define MAX_DEPTH 5


typedef struct
{
    float r, g, b;
} FloatColor;


double get_time()
{
    return (double)clock() / CLOCKS_PER_SEC;
}

// Plotting the graph img with sdl renderer, whose data was generated in case 1 (benchmark testing)
// Generated data was orginally plotted with gnuplot and saved to png
void display_plot_with_sdl(SDL_Renderer *renderer)
{

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return;
    }

    SDL_Surface *plot_surface = IMG_Load("benchmark_results.png");
    if (!plot_surface)
    {
        printf("Error loading plot: %s\n", IMG_GetError());
        return;
    }

    SDL_Texture *plot_texture = SDL_CreateTextureFromSurface(renderer, plot_surface);
    SDL_FreeSurface(plot_surface);

    if (!plot_texture)
    {
        printf("Error creating texture: %s\n", SDL_GetError());
        return;
    }

    int width, height;
    SDL_QueryTexture(plot_texture, NULL, NULL, &width, &height);
    SDL_Rect dest_rect = {
        .x = (WIDTH - width) / 2,
        .y = (HEIGHT - height) / 2,
        .w = width,
        .h = height};

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, plot_texture, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
    SDL_DestroyTexture(plot_texture);
    IMG_Quit();
}

//----------------------------------------------------------------------------------------------------

// MAIN

//----------------------------------------------------------------------------------------------------

#ifdef __APPLE__
int main(int argc, char *argv[])
#else
int SDL_main(int argc, char *argv[])
#endif
{

    srand(time(NULL));

    printf("\nPlease proceed as follows :\n\n");
    printf("Press '1' for benchmark testing with graph plot.\n");
    printf("Press '2' for Realtime CPU Raytracing.\n");
    printf("Waiting for the input : ");

    int input;
    scanf("%d", &input);

    switch (input)
    {

        //----------------------------------------------------------------------------------------------------

        // Benchmark Testing
        // Runs testing defined in benchmark.c through function call - run_benchmark_with_plotting();
        // Testing is done for the intersection of rays with the randomly generated spheres.
        // It involves comparison in tests between bvh and no bvh implementation.
        // The bvh implementation might show constant time run, as the test cases are not enough.
        // Result is plotted with gnuplot(c plotting library) and saved as png, which is further rendered
        // with sdl2 (windowing library uses in this project). Plot data is generated in plot_benchmark.gnu

        //----------------------------------------------------------------------------------------------------

    case 1:
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }

        SDL_Window *window = SDL_CreateWindow("Testing",
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
        if (!window)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer)
        {
            printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        run_benchmark_with_plotting();
        if (renderer)
        {
            display_plot_with_sdl(renderer);
            SDL_Event e;
            while (SDL_WaitEvent(&e))
            {
                if (e.type == SDL_QUIT ||
                    (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
                {
                    break;
                }
            }
        }

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        break;
    }
        //----------------------------------------------------------------------------------------------------

        // Realtime Raytracing on CPU
        // Scene has only spheres for now, adding custom mesh is not yet supported
        // Both BVH and without can be used for rendering the scene (not much
        // difference would be observed for less number of spheres, and cpu would not be
        // able to render high number of spheres, so kindly use benchmark testing to see the difference)
        // Number of spheres can be defined in constants header file (include/Custom/constants.h)
        // Camera has been added for moving in the defined scene
        // Debug mode is added to see Bounding Volumes by pressing 'o'
        // but its projection is still not properly aligned which needs to be corrected

        //----------------------------------------------------------------------------------------------------

    case 2:
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }

        SDL_Window *window = SDL_CreateWindow("Raytracer",
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
        if (!window)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }

        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                    SDL_RENDERER_PRESENTVSYNC);
        if (!renderer)
        {
            printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }

        Camera camera = {
            .position = {0, 4, 50},
            .forward = {0, 0, -1},
            .right = {1, 0, 0},
            .up = {0, 1, 0},
            .yaw = -M_PI,
            .pitch = 0,
            .fov = 45.0f,
            .move = 0};

        Sphere spheres[NUM_SPHERES];

        // spheres[0] = create_sphere(((Vec3){0.0f, -100.0f, 30.0f}), 100.0f);
        // spheres[1] = create_sphere(((Vec3){0.0f, 3.0f, 30.0f}), 3.0f);

        for (int i = 0; i < NUM_SPHERES; i++)
        {
            spheres[i] = create_random_sphere();
        }

        printf("Building BVH...\n");
        double bvh_start = get_time();
        BVHNode *root = build_bvh_node(spheres, 0, NUM_SPHERES, 0);
        double bvh_end = get_time();
        double bvh_build_time = bvh_end - bvh_start;
        printf("BVH built in %f seconds\n", bvh_build_time);

        int quit = 0;
        SDL_Event e;

        int frame_count = 0;
        double frame_start, frame_end;
        double total_render_time = 0;

        int use_bvh = 1;
        int show_bvh_visualization = 0;

        int accumulated_frames = 1;
        FloatColor **accumulated_colors = (FloatColor **)malloc(WIDTH * sizeof(FloatColor *));
        if (!accumulated_colors)
        {
            printf("Failed to allocate memory for accumulated_colors\n");
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
        for (int i = 0; i < WIDTH; i++)
        {
            accumulated_colors[i] = (FloatColor *)malloc(HEIGHT * sizeof(FloatColor));
            if (!accumulated_colors[i])
            {
                printf("Failed to allocate memory for accumulated_colors[%d]\n", i);

                for (int j = 0; j < i; j++)
                {
                    free(accumulated_colors[j]);
                }
                free(accumulated_colors);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 1;
            }
            for (int j = 0; j < HEIGHT; j++)
            {
                accumulated_colors[i][j].r = 0.0f;
                accumulated_colors[i][j].g = 0.0f;
                accumulated_colors[i][j].b = 0.0f;
            }
        }
        while (!quit)
        {

            frame_start = get_time();
            while (SDL_PollEvent(&e) != 0)
            {

                if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
                {
                    quit = 1;
                }
                else if (e.type == SDL_KEYDOWN)
                {
                    switch (e.key.keysym.sym)
                    {
                    case SDLK_w:
                        camera.position = vec3_add(camera.position,
                                                   vec3_multiply(camera.forward, MOVE_SPEED));
                        camera.move = 1;
                        break;
                    case SDLK_s:
                        camera.position = vec3_sub(camera.position,
                                                   vec3_multiply(camera.forward, MOVE_SPEED));
                        camera.move = 1;
                        break;
                    case SDLK_a:
                        camera.position = vec3_sub(camera.position,
                                                   vec3_multiply(camera.right, MOVE_SPEED));
                        camera.move = 1;
                        break;
                    case SDLK_d:
                        camera.position = vec3_add(camera.position,
                                                   vec3_multiply(camera.right, MOVE_SPEED));
                        camera.move = 1;
                        break;
                    case SDLK_SPACE:
                        camera.position.y += MOVE_SPEED;
                        camera.move = 1;
                        break;
                    case SDLK_LSHIFT:
                        camera.position.y -= MOVE_SPEED;
                        camera.move = 1;
                        break;
                    case SDLK_b:
                        use_bvh = !use_bvh;
                        printf("BVH %s\n", use_bvh ? "enabled" : "disabled");
                        break;
                    case SDLK_o:
                        show_bvh_visualization = !show_bvh_visualization;
                        printf("BVH visualization %s\n", show_bvh_visualization ? "enabled" : "disabled");
                        break;
                    }
                }
                else if (e.type == SDL_MOUSEMOTION)
                {
                    if (e.motion.state & SDL_BUTTON_LMASK)
                    {
                        camera.yaw += e.motion.xrel * ROTATE_SPEED;
                        camera.pitch -= e.motion.yrel * ROTATE_SPEED;
                        camera.pitch = fmax(fmin(camera.pitch, M_PI / 2 - 0.1f), -M_PI / 2 + 0.1f);
                        camera_update(&camera);
                        camera.move = 1;
                    }
                }
            }
            if (show_bvh_visualization)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);

                render_debug_visualization(renderer, root, &camera);
                SDL_RenderPresent(renderer);
            }
            else
            {

                if (camera.move)
                {

                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);

                    float aspect_ratio = (float)WIDTH / (float)HEIGHT;

                    for (int y = 0; y < HEIGHT; y++)
                    {
                        for (int x = 0; x < WIDTH; x++)
                        {
                            float u = ((float)x / WIDTH - 0.5f) * aspect_ratio;
                            float v = (float)y / HEIGHT - 0.5f;

                            Ray ray = get_camera_ray(&camera, u, -v);
                            SDL_Color color = trace_ray(ray, spheres, NUM_SPHERES, MAX_DEPTH, use_bvh ? root : NULL);

                            accumulated_colors[x][y].r = (float)color.r / 255.0f;
                            accumulated_colors[x][y].g = (float)color.g / 255.0f;
                            accumulated_colors[x][y].b = (float)color.b / 255.0f;
                            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                            SDL_RenderDrawPoint(renderer, x, y);
                        }
                    }

                    accumulated_frames = 1;
                    camera.move = 0;
                }
                else
                {
                    accumulated_frames++;
                    float aspect_ratio = (float)WIDTH / (float)HEIGHT;

                    for (int y = 0; y < HEIGHT; y++)
                    {
                        for (int x = 0; x < WIDTH; x++)
                        {
                            float u = ((float)x / WIDTH - 0.5f) * aspect_ratio;
                            float v = (float)y / HEIGHT - 0.5f;

                            Ray ray = get_camera_ray(&camera, u, -v);
                            SDL_Color color = trace_ray(ray, spheres, NUM_SPHERES, MAX_DEPTH, use_bvh ? root : NULL);

                            accumulated_colors[x][y].r += (float)color.r / 255.0f;
                            accumulated_colors[x][y].g += (float)color.g / 255.0f;
                            accumulated_colors[x][y].b += (float)color.b / 255.0f;

                            SDL_Color avg_color = {
                                (Uint8)(fmin(accumulated_colors[x][y].r / accumulated_frames * 255.0f, 255.0f)),
                                (Uint8)(fmin(accumulated_colors[x][y].g / accumulated_frames * 255.0f, 255.0f)),
                                (Uint8)(fmin(accumulated_colors[x][y].b / accumulated_frames * 255.0f, 255.0f)),
                                255};

                            SDL_SetRenderDrawColor(renderer, avg_color.r, avg_color.g, avg_color.b, avg_color.a);
                            SDL_RenderDrawPoint(renderer, x, y);
                        }
                    }
                }
                SDL_RenderPresent(renderer);

                frame_end = get_time();
                double frame_time = frame_end - frame_start;
                total_render_time += frame_time;
                frame_count++;

                if (frame_count % 10 == 0)
                {
                    printf("Average frame time: %f seconds (%.2f FPS)\n",
                           total_render_time / frame_count,
                           frame_count / total_render_time);
                }
            }
        }

        printf("\nFinal Performance Report:\n");
        printf("Total frames: %d\n", frame_count);
        printf("Average frame time: %f seconds\n", total_render_time / frame_count);
        printf("Average FPS: %.2f\n", frame_count / total_render_time);
        printf("BVH build time: %f seconds\n", bvh_build_time);

        for (int i = 0; i < WIDTH; i++)
        {
            free(accumulated_colors[i]);
        }
        free(accumulated_colors);

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        break;
    }
        //------------------------------------------------------------------------------------------

    default:
        printf("Please press only among the given options");
        break;
    }

    return 0;
}