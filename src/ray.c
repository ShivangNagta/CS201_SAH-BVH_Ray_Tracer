#include "Custom/ray.h"
#include "Custom/camera.h"
#include "Custom/constants.h"
#include <math.h>

//--------------------------------------------------------------------------------------------------

// Defines the direction of ray from camera's position depending upon camera's orientation
// Ray struct - 
// - Vec3 origin
// - Vec3 direction

//--------------------------------------------------------------------------------------------------



Ray get_camera_ray(Camera *camera, float u, float v) {
    float aspect_ratio = (float)WIDTH / (float)HEIGHT;
    float fov_rad = camera->fov * (M_PI / 180.0f);
    float half_height = tan(fov_rad / 2.0f);
    float half_width = aspect_ratio * half_height;

    Vec3 horizontal = vec3_multiply(camera->right, 2.0f * half_width);
    Vec3 vertical = vec3_multiply(camera->up, 2.0f * half_height);
    
    Vec3 direction = camera->forward;
    direction = vec3_add(direction, vec3_multiply(horizontal, u));
    direction = vec3_add(direction, vec3_multiply(vertical, v));
    direction = vec3_normalize(direction);
    
    return (Ray){camera->position, direction};
}


// Ray get_camera_ray(Camera *camera, float u, float v) {
//     float aspect_ratio = (float)WIDTH / (float)HEIGHT;
//     float fov_rad = camera->fov * (M_PI / 180.0f);
//     float half_height = tan(fov_rad / 2.0f);
//     float half_width = aspect_ratio * half_height;

   
//     float adjusted_u = (2.0f * u - 1.0f) * half_width;
//     float adjusted_v = (2.0f * v - 1.0f) * half_height;
    
//     Vec3 direction = camera->forward;
//     direction = vec3_add(direction, vec3_multiply(camera->right, adjusted_u));
//     direction = vec3_add(direction, vec3_multiply(camera->up, adjusted_v));
//     direction = vec3_normalize(direction);
    
//     return (Ray){camera->position, direction};
// }