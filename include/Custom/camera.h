#pragma once

#include "Custom/vec3.h"

typedef struct {
    Vec3 position;
    Vec3 forward;
    Vec3 right;
    Vec3 up;
    float yaw;
    float pitch;
    int move;
} Camera;


void camera_update(Camera *camera);