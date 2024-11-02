// In bvh_visualizer.h
#ifndef BVH_VISUALIZER_H
#define BVH_VISUALIZER_H

#include <SDL2/SDL.h>
#include <math.h>
#include "Custom/camera.h"
#include "Custom/bvh.h"
#include "Custom/vec3.h"

// Improved world to screen projection
static SDL_Point world_to_screen(Vec3 point, Camera* camera, int screen_width, int screen_height) {
    // Transform point to camera space
    Vec3 to_point = vec3_sub(point, camera->position);
    
    // Project point into camera space using camera orientation vectors
    float x = vec3_dot(to_point, camera->right);
    float y = vec3_dot(to_point, camera->up);
    float z = vec3_dot(to_point, camera->forward);
    
    // Frustum culling - return invalid point if behind camera
    if (z <= 0.1f) {
        return (SDL_Point){-1, -1};
    }
    
    // Perspective projection parameters
    float fov = 60.0f * M_PI / 180.0f;  // 60 degree FOV
    float near_plane = 0.1f;
    float aspect = (float)screen_width / screen_height;
    
    // Calculate perspective projection
    float tan_half_fov = tanf(fov * 0.5f);
    float screen_x = (x / (z * tan_half_fov * aspect));
    float screen_y = (y / (z * tan_half_fov));
    
    // Convert to screen space coordinates
    int pixel_x = (int)((screen_x + 1.0f) * 0.5f * screen_width);
    int pixel_y = (int)((1.0f - (screen_y + 1.0f) * 0.5f) * screen_height);
    
    // Frustum culling for screen space
    if (pixel_x < -100 || pixel_x > screen_width + 100 ||
        pixel_y < -100 || pixel_y > screen_height + 100) {
        return (SDL_Point){-1, -1};
    }
    
    return (SDL_Point){pixel_x, pixel_y};
}

// Improved line drawing with clipping
static void draw_debug_line(SDL_Renderer* renderer, Vec3 start, Vec3 end, 
                          Camera* camera, int screen_width, int screen_height) {
    SDL_Point start_screen = world_to_screen(start, camera, screen_width, screen_height);
    SDL_Point end_screen = world_to_screen(end, camera, screen_width, screen_height);
    
    // Skip if either point is invalid
    if (start_screen.x == -1 || end_screen.x == -1) return;
    
    // Draw thicker lines with better visibility
    const int thickness = 2;
    for(int dx = -thickness/2; dx <= thickness/2; dx++) {
        for(int dy = -thickness/2; dy <= thickness/2; dy++) {
            SDL_RenderDrawLine(renderer,
                             start_screen.x + dx, start_screen.y + dy,
                             end_screen.x + dx, end_screen.y + dy);
        }
    }
}

// Improved AABB drawing with depth-based size adjustment
static void draw_aabb(SDL_Renderer* renderer, AABB box, Camera* camera, 
                     int screen_width, int screen_height) {
    // Calculate center and distance to camera for depth-based rendering
    Vec3 center = {
        (box.min.x + box.max.x) * 0.5f,
        (box.min.y + box.max.y) * 0.5f,
        (box.min.z + box.max.z) * 0.5f
    };
    
    Vec3 to_center = vec3_sub(center, camera->position);
    float dist_to_camera = vec3_len(to_center);
    
    // Skip if too far or too close
    if (dist_to_camera > 1000.0f || dist_to_camera < 0.1f) return;
    
    Vec3 corners[8] = {
        {box.min.x, box.min.y, box.min.z}, // 0
        {box.max.x, box.min.y, box.min.z}, // 1
        {box.max.x, box.max.y, box.min.z}, // 2
        {box.min.x, box.max.y, box.min.z}, // 3
        {box.min.x, box.min.y, box.max.z}, // 4
        {box.max.x, box.min.y, box.max.z}, // 5
        {box.max.x, box.max.y, box.max.z}, // 6
        {box.min.x, box.max.y, box.max.z}  // 7
    };
    
    // Front face
    draw_debug_line(renderer, corners[0], corners[1], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[1], corners[2], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[2], corners[3], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[3], corners[0], camera, screen_width, screen_height);
    
    // Back face
    draw_debug_line(renderer, corners[4], corners[5], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[5], corners[6], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[6], corners[7], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[7], corners[4], camera, screen_width, screen_height);
    
    // Connecting lines
    draw_debug_line(renderer, corners[0], corners[4], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[1], corners[5], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[2], corners[6], camera, screen_width, screen_height);
    draw_debug_line(renderer, corners[3], corners[7], camera, screen_width, screen_height);
}

// Improved BVH recursive drawing with better depth handling
static void draw_bvh_recursive(SDL_Renderer* renderer, BVHNode* node, Camera* camera, 
                             int screen_width, int screen_height, int depth) {
    if (!node) return;
    
    // Calculate node center and distance to camera
    Vec3 center = {
        (node->bounds.min.x + node->bounds.max.x) * 0.5f,
        (node->bounds.min.y + node->bounds.max.y) * 0.5f,
        (node->bounds.min.z + node->bounds.max.z) * 0.5f
    };
    
    Vec3 to_center = vec3_sub(center, camera->position);
    float dist_to_camera = vec3_len(to_center);
    
    // Skip if too far or too close
    if (dist_to_camera > 1000.0f || dist_to_camera < 0.1f) return;

    // Depth-based color with distance-based alpha
    Uint8 r = 255 - (depth * 40) % 200;
    Uint8 g = (depth * 80) % 200;
    Uint8 b = (depth * 120) % 200;
    Uint8 alpha = (Uint8)(fmaxf(0.0f, fminf(255.0f, 255.0f * (50.0f / dist_to_camera))));
    
    SDL_SetRenderDrawColor(renderer, r, g, b, alpha);
    draw_aabb(renderer, node->bounds, camera, screen_width, screen_height);
    
    // Draw children
    if (node->left) draw_bvh_recursive(renderer, node->left, camera, screen_width, screen_height, depth + 1);
    if (node->right) draw_bvh_recursive(renderer, node->right, camera, screen_width, screen_height, depth + 1);
}

// Main visualization function with improved rendering settings
void render_debug_visualization(SDL_Renderer* renderer, BVHNode* root, Camera* camera) {
    if (!root) return;
    
    int screen_width, screen_height;
    SDL_GetRendererOutputSize(renderer, &screen_width, &screen_height);
    
    // Enable blending for transparent wireframes
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Draw BVH structure
    draw_bvh_recursive(renderer, root, camera, screen_width, screen_height, 0);
    
    // Reset blend mode
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

#endif // BVH_VISUALIZER_H