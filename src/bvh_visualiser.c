#include <SDL2/SDL.h>
#include <math.h>
#include <stdio.h>
#include "Custom/camera.h"
#include "Custom/bvh.h"
#include "Custom/vec3.h"

//-----------------------------------------------------------------------------------------------------

// NOT WORKING PROPERLY AS OF NOW

// Debug mode for visualising Bounding Boxes (Switch it on by pressing 'o' in Raytracing mode)

//-----------------------------------------------------------------------------------------------------

static SDL_Point world_to_screen(Vec3 point, Camera* camera, int screen_width, int screen_height) {

    Vec3 to_point = vec3_sub(point, camera->position);
    float z = vec3_dot(to_point, camera->forward);
    if (z <= 0.1f) {
        return (SDL_Point){-1, -1}; 
    }
    
    float x = vec3_dot(to_point, camera->right);
    float y = vec3_dot(to_point, camera->up);
    
    float fov_rad = camera->fov * (M_PI / 180.0f);
    float half_height = tanf(fov_rad / 2.0f);
    float aspect_ratio = (float)screen_width / screen_height;
    float half_width = aspect_ratio * half_height;

    float screen_x = (x / (z * half_width * 2.0f) + 0.5f) * screen_width;
    float screen_y = (-y / (z * half_height * 2.0f) + 0.5f) * screen_height;
    
    if (screen_x < -screen_width || screen_x > screen_width * 2 ||
        screen_y < -screen_height || screen_y > screen_height * 2) {
        return (SDL_Point){-1, -1};
    }
    
    return (SDL_Point){(int)screen_x, (int)screen_y};
}

static void draw_debug_line(SDL_Renderer* renderer, Vec3 start, Vec3 end, 
                          Camera* camera, int screen_width, int screen_height) {
    SDL_Point start_screen = world_to_screen(start, camera, screen_width, screen_height);
    SDL_Point end_screen = world_to_screen(end, camera, screen_width, screen_height);
    
    if (start_screen.x == -1 || end_screen.x == -1) return;
    
    if (start_screen.x >= -screen_width && start_screen.x <= screen_width * 2 &&
        start_screen.y >= -screen_height && start_screen.y <= screen_height * 2 &&
        end_screen.x >= -screen_width && end_screen.x <= screen_width * 2 &&
        end_screen.y >= -screen_height && end_screen.y <= screen_height * 2) {
        
        SDL_RenderDrawLine(renderer, start_screen.x, start_screen.y,
                          end_screen.x, end_screen.y);
        SDL_RenderDrawLine(renderer, start_screen.x+1, start_screen.y,
                          end_screen.x+1, end_screen.y);
        SDL_RenderDrawLine(renderer, start_screen.x, start_screen.y+1,
                          end_screen.x, end_screen.y+1);
        SDL_RenderDrawLine(renderer, start_screen.x-1, start_screen.y,
                          end_screen.x-1, end_screen.y);
        SDL_RenderDrawLine(renderer, start_screen.x, start_screen.y-1,
                          end_screen.x, end_screen.y-1);
    }
}


static void draw_aabb(SDL_Renderer* renderer, AABB box, Camera* camera, 
                     int screen_width, int screen_height) {
    Vec3 corners[8] = {
        {box.min.x, box.min.y, box.min.z}, 
        {box.max.x, box.min.y, box.min.z}, 
        {box.max.x, box.max.y, box.min.z}, 
        {box.min.x, box.max.y, box.min.z}, 
        {box.min.x, box.min.y, box.max.z}, 
        {box.max.x, box.min.y, box.max.z}, 
        {box.max.x, box.max.y, box.max.z}, 
        {box.min.x, box.max.y, box.max.z}  
    };
    
    draw_debug_line(renderer, corners[0], corners[1], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[1], corners[2], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[2], corners[3], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[3], corners[0], camera, screen_width, screen_height);
    
    draw_debug_line(renderer, corners[4], corners[5], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[5], corners[6], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[6], corners[7], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[7], corners[4], camera, screen_width, screen_height);
    
    draw_debug_line(renderer, corners[0], corners[4], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[1], corners[5], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[2], corners[6], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[3], corners[7], camera, screen_width, screen_height);
}


void draw_bvh_recursive(SDL_Renderer* renderer, BVHNode* node, Camera* camera, 
                       int screen_width, int screen_height, int depth) {
    if (!node) return;
    
    Uint8 r = 255 - (depth * 40) % 200;
    Uint8 g = (depth * 80) % 200;
    Uint8 b = (depth * 120) % 200;
    SDL_SetRenderDrawColor(renderer, r, g, b, 180);
    draw_aabb(renderer, node->bounds, camera, screen_width, screen_height);
    
    
    if (node->left) draw_bvh_recursive(renderer, node->left, camera, screen_width, screen_height, depth + 1);
    if (node->right) draw_bvh_recursive(renderer, node->right, camera, screen_width, screen_height, depth + 1);
}


void render_debug_visualization(SDL_Renderer* renderer, BVHNode* root, Camera* camera) {

    if (!root) {
        printf("No BVH root node provided!\n");
        return;
    }
    int screen_width, screen_height;
    SDL_GetRendererOutputSize(renderer, &screen_width, &screen_height);
    draw_bvh_recursive(renderer, root, camera, screen_width, screen_height, 0);
    

}

// void draw_camera_debug(SDL_Renderer* renderer, Camera* camera, int screen_width, int screen_height) {
//     // Increase the debug line length to make them more visible
//     float debug_length = 5.0f;  // Increased from 1.0f to 5.0f
    
//     // Calculate endpoints for the camera's axes
//     Vec3 forward_end = vec3_add(camera->position, vec3_multiply(camera->forward, debug_length));
//     Vec3 right_end = vec3_add(camera->position, vec3_multiply(camera->right, debug_length));
//     Vec3 up_end = vec3_add(camera->position, vec3_multiply(camera->up, debug_length));
    
//     // Draw a small point at camera position
//     SDL_Point camera_pos = world_to_screen(camera->position, camera, screen_width, screen_height);
//     if (camera_pos.x != -1) {
//         // Draw a small cross at camera position
//         SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // White for camera position
//         SDL_RenderDrawLine(renderer, camera_pos.x - 5, camera_pos.y, camera_pos.x + 5, camera_pos.y);
//         SDL_RenderDrawLine(renderer, camera_pos.x, camera_pos.y - 5, camera_pos.x, camera_pos.y + 5);
//     }
    
//     // Draw the axis lines with thicker rendering
//     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red for forward
//     draw_debug_line(renderer, camera->position, forward_end, camera, screen_width, screen_height);
    
//     SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green for right
//     draw_debug_line(renderer, camera->position, right_end, camera, screen_width, screen_height);
    
//     SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Blue for up
//     draw_debug_line(renderer, camera->position, up_end, camera, screen_width, screen_height);
// }
