#include "Custom/hit.h"
#include "Custom/vec3.h"
#include "Custom/constants.h"
#include <math.h>

HitRecord ray_sphere_intersect(Ray ray, Sphere *sphere) {
    HitRecord rec = {0};
    Vec3 oc = vec3_sub(ray.origin, sphere->center);
    float a = vec3_dot(ray.direction, ray.direction);
    float b = 2.0f * vec3_dot(oc, ray.direction);
    float c = vec3_dot(oc, oc) - sphere->radius * sphere->radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant > 0) {
        float t = (-b - sqrt(discriminant)) / (2.0f * a);
        if (t > EPSILON) {
            rec.hit_something = 1;
            rec.t = t;
            rec.point = vec3_add(ray.origin, vec3_multiply(ray.direction, t));
            rec.normal = vec3_normalize(vec3_sub(rec.point, sphere->center));
            rec.object = sphere;
            return rec;
        }
    }
    return rec;
}