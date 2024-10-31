#include "Custom/ray.h"
#include "Custom/camera.h"

Ray get_camera_ray(Camera *camera, float u, float v) {
    Vec3 horizontal = vec3_multiply(camera->right, 2.0f);
    Vec3 vertical = vec3_multiply(camera->up, 2.0f);
    
    Vec3 direction = camera->forward;
    direction = vec3_add(direction, vec3_multiply(horizontal, u));
    direction = vec3_add(direction, vec3_multiply(vertical, v));
    direction = vec3_normalize(direction);
    
    return (Ray){camera->position, direction};
}