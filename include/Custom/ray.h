#pragma once
#include "Custom/vec3.h"
#include "Custom/camera.h"

typedef struct {
    Vec3 origin;
    Vec3 direction;
} Ray;


Ray get_camera_ray(Camera *camera, float u, float v);

