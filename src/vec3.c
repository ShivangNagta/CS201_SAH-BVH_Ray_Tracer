#include <math.h>
#include <stdlib.h>
#include "Custom/vec3.h"

//--------------------------------------------------------------------------------------------------

// Helper vector3 functions
// Vec3 struct -
// float x
// float y
// float z

//--------------------------------------------------------------------------------------------------



Vec3 vec3_sub(Vec3 a, Vec3 b){
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
};

Vec3 vec3_normalize(Vec3 a){
    float len = sqrt(a.x*a.x + a.y*a.y + a.z *a.z);
    return (len != 0) ? (Vec3){a.x / len, a.y / len, a.z / len} : (Vec3){0, 0, 0};
};

float vec3_dot(Vec3 a, Vec3 b){
    return a.x*b.x + a.y * b.y + a.z * b.z;
};

Vec3 vec3_add(Vec3 a, Vec3 b){
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
};

Vec3 vec3_multiply(Vec3 v, float t) {
    return (Vec3){v.x * t, v.y * t, v.z * t};
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 vec3_reflect(Vec3 v, Vec3 n) {
    float dot = vec3_dot(v, n);
    return vec3_sub(v, vec3_multiply(n, 2 * dot));
}

Vec3 vec3_refract(Vec3 uv, Vec3 n, float etai_over_etat) {
    float cos_theta = fmin(vec3_dot(vec3_multiply(uv, -1), n), 1.0);
    Vec3 r_out_perp = vec3_multiply(
        vec3_add(uv, vec3_multiply(n, cos_theta)),
        etai_over_etat
    );
    Vec3 r_out_parallel = vec3_multiply(
        n,
        -sqrt(fabs(1.0 - vec3_dot(r_out_perp, r_out_perp)))
    );
    return vec3_add(r_out_perp, r_out_parallel);
}

Vec3 vec3_random(float min, float max){
    float x = min + (float)rand() / RAND_MAX * (max - min);
    float y = min + (float)rand() / RAND_MAX * (max - min);
    float z = min + (float)rand() / RAND_MAX * (max - min);
    return (Vec3){x, y, z};
}

float vec3_len(Vec3 a){
    return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}