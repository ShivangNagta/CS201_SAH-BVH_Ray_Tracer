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

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include <Nuklear/nuklear.h>
#include <Nuklear/nuklear_sdl_renderer.h>

#define NUM_SPHERES 3
#define WIDTH 1000
#define HEIGHT 600

// Helper function for time calculations
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
    printf("Press '3' for Static Rendering comparison with and without optimisation. (TODO) \n");
    printf("Press '4' for visulisation of Bounding Volume Hierarchies on mesh object. (TODO) \n\n");
    printf("Waiting for the input : ");

    int input;
    scanf("%d", &input);

    switch (input)
    {

        //----------------------------------------------------------------------------------------------------

        // Benchmark Testing
        // Runs testing defined in benchmark.c through function call - run_benchmark_with_plotting();
        // Testing is done for the intersection of rays with the randomly generated spheres.

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
        // Both with and without BVH can be used for rendering the scene (not much
        // difference would be observed for less number of spheres, and cpu would not
        // able to render high number of spheres, so kindly use benchmark testing to see the difference)
        // Number of spheres can be defined in constants header file (include/Custom/constants.h)
        // Camera has been added for moving in the defined scene

    case 2:
    {

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
            return 1;
        }

        SDL_Window *window = SDL_CreateWindow("Raytracer with GUI",
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

        struct nk_context *ctx = nk_sdl_init(window, renderer);
        float font_scale = 1;

    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        /* set up the font atlas and add desired font; note that font sizes are
         * multiplied by font_scale to produce better results at higher DPIs */
        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10 * font_scale, &config);*/
        /*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13 * font_scale, &config);*/
        nk_sdl_font_stash_end();

        /* this hack makes the font appear to be scaled down to the desired
         * size and is only necessary when font_scale > 1 */
        font->handle.height /= font_scale;
        /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
        nk_style_set_font(ctx, &font->handle);
    }

        if (!ctx) {
        printf("Nuklear context initialization failed!\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return 1;
}

        Camera camera = {
            .position = {2, 4, 5},
            .forward = {0, 0, -1},
            .right = {1, 0, 0},
            .up = {0, 1, 0},
            .yaw = -M_PI,
            .pitch = 0};

        Sphere spheres[3];

        spheres[0] = create_sphere(((Vec3){0.0f, 0.0f, -10.0f}), 25.0f);
        spheres[1] = create_sphere(((Vec3){10.0f, 0.0f, -5.0f}), 3.0f);
        spheres[2] = create_sphere(((Vec3){-10.0f, 0.0f, -5.0f}), 3.0f);

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

        while (!quit)
        {
            nk_input_begin(ctx);
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
                else if (e.type == SDL_MOUSEMOTION)
                {
                    if (e.motion.state & SDL_BUTTON_LMASK)
                    {
                        camera.yaw += e.motion.xrel * ROTATE_SPEED;
                        camera.pitch -= e.motion.yrel * ROTATE_SPEED;
                        camera.pitch = fmax(fmin(camera.pitch, M_PI / 2 - 0.1f), -M_PI / 2 + 0.1f);
                        camera_update(&camera);
                    }
                }
                nk_sdl_handle_event(&e);
            }
            nk_input_end(ctx);

            frame_start = get_time();

            // Nuklear GUI setup
            if (nk_begin(ctx, "Sphere Controls", nk_rect(0, 0, 200, HEIGHT),
                         NK_WINDOW_BORDER | NK_WINDOW_TITLE))
            {
                for (int i = 0; i < 3; i++)
                {
                    char label[32];
                    snprintf(label, sizeof(label), "Sphere %d Position", i + 1);
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_label(ctx, label, NK_TEXT_CENTERED);

                    nk_layout_row_dynamic(ctx, 25, 2);
                    nk_property_float(ctx, "X:", -20.0f, &spheres[i].center.x, 200.0f, 1.0f, 0.01f);
                    nk_property_float(ctx, "Y:", -20.0f, &spheres[i].center.y, 200.0f, 1.0f, 0.01f);
                    nk_property_float(ctx, "Z:", -20.0f, &spheres[i].center.z, 200.0f, 1.0f, 0.01f);

                    snprintf(label, sizeof(label), "Sphere %d Radius", i + 1);
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_property_float(ctx, label, 0.1f, &spheres[i].radius, 100.0f, 1.0f, 0.01f);
                }
            }
            nk_end(ctx);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            // Raytracing loop
            for (int y = 0; y < HEIGHT; y++)
            {
                for (int x = 0; x < WIDTH; x++)
                {
                    float u = (float)x / WIDTH - 0.5f;
                    float v = (float)y / HEIGHT - 0.5f;

                    Ray ray = get_camera_ray(&camera, u, -v);
                    SDL_Color color = trace_ray(ray, spheres, NUM_SPHERES, MAX_DEPTH,
                                                use_bvh ? root : NULL);

                    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                    SDL_RenderDrawPoint(renderer, x, y);
                }
            }

            // Render Nuklear GUI
            nk_sdl_render(NK_ANTI_ALIASING_ON);

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

        printf("\nFinal Performance Report:\n");
        printf("Total frames: %d\n", frame_count);
        printf("Average frame time: %f seconds\n", total_render_time / frame_count);
        printf("Average FPS: %.2f\n", frame_count / total_render_time);
        printf("BVH build time: %f seconds\n", bvh_build_time);

        nk_sdl_shutdown();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        break;
    

        // if (SDL_Init(SDL_INIT_VIDEO) < 0)
        // {
        //     printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        //     return 1;
        // }

        // SDL_Window *window = SDL_CreateWindow("Raytracer",
        //                                       SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        //                                       WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
        // if (!window)
        // {
        //     printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        //     return 1;
        // }

        // SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        //                                             SDL_RENDERER_PRESENTVSYNC);
        // if (!renderer)
        // {
        //     printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        //     SDL_DestroyWindow(window);
        //     SDL_Quit();
        //     return 1;
        // }

        // Camera camera = {
        //     .position = {2, 4, 5},
        //     .forward = {0, 0, -1},
        //     .right = {1, 0, 0},
        //     .up = {0, 1, 0},
        //     .yaw = -M_PI,
        //     .pitch = 0};

        // Sphere spheres[3];

        // spheres[0] = create_sphere(((Vec3){0.0f, 0.0f, -10.0f}), 5.0f);
        // spheres[1] = create_sphere(((Vec3){10.0f, 0.0f, -5.0f}), 3.0f);
        // spheres[2] = create_sphere(((Vec3){-10.0f, 0.0f, -5.0f}), 3.0f);

        // // for (int i = 2; i < NUM_SPHERES; i++)
        // // {
        // //     spheres[i] = create_random_sphere(0);
        // // }

        // printf("Building BVH...\n");
        // double bvh_start = get_time();
        // BVHNode *root = build_bvh_node(spheres, 0, NUM_SPHERES, 0);
        // double bvh_end = get_time();
        // double bvh_build_time = bvh_end - bvh_start;
        // printf("BVH built in %f seconds\n", bvh_build_time);

        // int quit = 0;
        // SDL_Event e;

        // int frame_count = 0;
        // double frame_start, frame_end;
        // double total_render_time = 0;

        // int use_bvh = 1;

        // while (!quit)
        // {
        //     while (SDL_PollEvent(&e) != 0)
        //     {
        //         if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
        //         {
        //             quit = 1;
        //         }
        //         else if (e.type == SDL_KEYDOWN)
        //         {
        //             switch (e.key.keysym.sym)
        //             {
        //             case SDLK_w:
        //                 camera.position = vec3_add(camera.position,
        //                                            vec3_multiply(camera.forward, MOVE_SPEED));
        //                 break;
        //             case SDLK_s:
        //                 camera.position = vec3_sub(camera.position,
        //                                            vec3_multiply(camera.forward, MOVE_SPEED));
        //                 break;
        //             case SDLK_a:
        //                 camera.position = vec3_sub(camera.position,
        //                                            vec3_multiply(camera.right, MOVE_SPEED));
        //                 break;
        //             case SDLK_d:
        //                 camera.position = vec3_add(camera.position,
        //                                            vec3_multiply(camera.right, MOVE_SPEED));
        //                 break;
        //             case SDLK_SPACE:
        //                 camera.position.y += MOVE_SPEED;
        //                 break;
        //             case SDLK_LSHIFT:
        //                 camera.position.y -= MOVE_SPEED;
        //                 break;
        //             case SDLK_b:
        //                 use_bvh = !use_bvh;
        //                 printf("BVH %s\n", use_bvh ? "enabled" : "disabled");
        //                 break;
        //             }
        //         }
        //         else if (e.type == SDL_MOUSEMOTION)
        //         {
        //             if (e.motion.state & SDL_BUTTON_LMASK)
        //             {
        //                 camera.yaw += e.motion.xrel * ROTATE_SPEED;
        //                 camera.pitch -= e.motion.yrel * ROTATE_SPEED;
        //                 camera.pitch = fmax(fmin(camera.pitch, M_PI / 2 - 0.1f), -M_PI / 2 + 0.1f);
        //                 camera_update(&camera);
        //             }
        //         }
        //     }
        //     frame_start = get_time();
        //     SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        //     SDL_RenderClear(renderer);

        //     for (int y = 0; y < HEIGHT; y++)
        //     {
        //         for (int x = 0; x < WIDTH; x++)
        //         {
        //             float u = (float)x / WIDTH - 0.5f;
        //             float v = (float)y / HEIGHT - 0.5f;

        //             Ray ray = get_camera_ray(&camera, u, -v);
        //             SDL_Color color = trace_ray(ray, spheres, NUM_SPHERES, MAX_DEPTH,
        //                                         use_bvh ? root : NULL);

        //             SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        //             SDL_RenderDrawPoint(renderer, x, y);
        //         }
        //     }

        //     SDL_RenderPresent(renderer);
        //     frame_end = get_time();
        //     double frame_time = frame_end - frame_start;
        //     total_render_time += frame_time;
        //     frame_count++;

        //     if (frame_count % 10 == 0)
        //     {
        //         printf("Average frame time: %f seconds (%.2f FPS)\n",
        //                total_render_time / frame_count,
        //                frame_count / total_render_time);
        //     }
        // }

        // printf("\nFinal Performance Report:\n");
        // printf("Total frames: %d\n", frame_count);
        // printf("Average frame time: %f seconds\n", total_render_time / frame_count);
        // printf("Average FPS: %.2f\n", frame_count / total_render_time);
        // printf("BVH build time: %f seconds\n", bvh_build_time);

        // SDL_DestroyRenderer(renderer);
        // SDL_DestroyWindow(window);
        // SDL_Quit();
        // break;
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

return 0;
}