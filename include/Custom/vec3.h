#pragma once

typedef struct Vec3{
    float x;
    float y;
    float z;
} Vec3;


Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_normalize(Vec3 a);
float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_multiply(Vec3 v, float t);
Vec3 vec3_cross(Vec3 a, Vec3 b);
Vec3 vec3_reflect(Vec3 v, Vec3 n);
Vec3 vec3_refract(Vec3 uv, Vec3 n, float etai_over_etat);
Vec3 vec3_random(float min, float max);
float vec3_len(Vec3 a);
