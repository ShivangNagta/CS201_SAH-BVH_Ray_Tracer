//A header only implementation for Vec3 operations

#pragma once
#include <math.h>


typedef struct Vec3{
    float x;
    float y;
    float z;
} Vec3;


inline Vec3 vec3_sub(Vec3 a, Vec3 b){
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
};

inline Vec3 vec3_normalize(Vec3 a){
    float len = sqrt(a.x*a.x + a.y*a.y + a.z *a.z);
    return (len != 0) ? (Vec3){a.x / len, a.y / len, a.z / len} : (Vec3){0, 0, 0};
};

inline float vec3_dot(Vec3 a, Vec3 b){
    return a.x*b.x + a.y * b.y + a.z * b.z;
};


