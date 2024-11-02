#include "Custom/camera.h"
#include <math.h>

//----------------------------------------------------------------------------------------------------

// Updates camera's orientation from user input

//----------------------------------------------------------------------------------------------------

void camera_update(Camera *camera) {
    
    camera->forward.x = cos(camera->pitch) * sin(camera->yaw);
    camera->forward.y = sin(camera->pitch);
    camera->forward.z = cos(camera->pitch) * cos(camera->yaw);
    camera->forward = vec3_normalize(camera->forward);
    camera->right = vec3_normalize(vec3_cross(camera->forward, (Vec3){0, 1, 0}));
    camera->up = vec3_normalize(vec3_cross(camera->right, camera->forward));
}