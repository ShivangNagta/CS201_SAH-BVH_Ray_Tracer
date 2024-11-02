#pragma once

#include <SDL2/SDL.h>
#include "Custom/bvh.h"
#include "Custom/ray.h"



void draw_bvh_recursive(SDL_Renderer* renderer, BVHNode* node, Camera* camera, 
                       int screen_width, int screen_height, int depth);
void render_debug_visualization(SDL_Renderer* renderer, BVHNode* root, Camera* camera);
void draw_camera_debug(SDL_Renderer* renderer, Camera* camera, int screen_width, int screen_height);

