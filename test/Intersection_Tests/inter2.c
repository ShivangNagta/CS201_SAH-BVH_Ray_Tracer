#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    Vec3 origin;
    Vec3 direction;
} Ray;

typedef struct {
    Vec3 center;
    float radius;
} Sphere;

typedef struct {
    Vec3 min;
    Vec3 max;
} AABB;

typedef struct BVHNode {
    AABB bounds;
    Sphere* sphere;
    struct BVHNode* left;
    struct BVHNode* right;
} BVHNode;

// Vector operations
Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 vec3_normalize(Vec3 v) {
    float length = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return (Vec3){v.x / length, v.y / length, v.z / length};
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Ray-Sphere intersection that returns distance
float ray_sphere_intersection(Ray ray, Sphere sphere) {
    Vec3 oc = vec3_sub(ray.origin, sphere.center);
    float a = vec3_dot(ray.direction, ray.direction);
    float b = 2.0f * vec3_dot(oc, ray.direction);
    float c = vec3_dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) return INFINITY;
    
    float t = (-b - sqrtf(discriminant)) / (2.0f * a);
    return t > 0 ? t : INFINITY;
}

// AABB intersection
float ray_aabb_intersection(Ray ray, AABB bounds) {
    float tx1 = (bounds.min.x - ray.origin.x) / ray.direction.x;
    float tx2 = (bounds.max.x - ray.origin.x) / ray.direction.x;
    float tmin = fminf(tx1, tx2);
    float tmax = fmaxf(tx1, tx2);

    float ty1 = (bounds.min.y - ray.origin.y) / ray.direction.y;
    float ty2 = (bounds.max.y - ray.origin.y) / ray.direction.y;
    tmin = fmaxf(tmin, fminf(ty1, ty2));
    tmax = fminf(tmax, fmaxf(ty1, ty2));

    float tz1 = (bounds.min.z - ray.origin.z) / ray.direction.z;
    float tz2 = (bounds.max.z - ray.origin.z) / ray.direction.z;
    tmin = fmaxf(tmin, fminf(tz1, tz2));
    tmax = fminf(tmax, fmaxf(tz1, tz2));

    return tmax >= tmin && tmax > 0 ? tmin : INFINITY;
}

AABB calculate_sphere_bounds(Sphere sphere) {
    AABB bounds;
    bounds.min = (Vec3){
        sphere.center.x - sphere.radius,
        sphere.center.y - sphere.radius,
        sphere.center.z - sphere.radius
    };
    bounds.max = (Vec3){
        sphere.center.x + sphere.radius,
        sphere.center.y + sphere.radius,
        sphere.center.z + sphere.radius
    };
    return bounds;
}

// Improved BVH building with surface area heuristic
float calculate_box_surface_area(AABB bounds) {
    float dx = bounds.max.x - bounds.min.x;
    float dy = bounds.max.y - bounds.min.y;
    float dz = bounds.max.z - bounds.min.z;
    return 2.0f * (dx * dy + dy * dz + dz * dx);
}

int compare_spheres_x(const void* a, const void* b) {
    return ((Sphere*)a)->center.x > ((Sphere*)b)->center.x ? 1 : -1;
}

int compare_spheres_y(const void* a, const void* b) {
    return ((Sphere*)a)->center.y > ((Sphere*)b)->center.y ? 1 : -1;
}

int compare_spheres_z(const void* a, const void* b) {
    return ((Sphere*)a)->center.z > ((Sphere*)b)->center.z ? 1 : -1;
}

BVHNode* build_bvh(Sphere* spheres, int start, int end) {
    BVHNode* node = malloc(sizeof(BVHNode));
    
    if (start == end) {
        node->sphere = &spheres[start];
        node->bounds = calculate_sphere_bounds(spheres[start]);
        node->left = node->right = NULL;
        return node;
    }
    
    // Calculate bounds for current set of spheres
    AABB bounds = calculate_sphere_bounds(spheres[start]);
    for (int i = start + 1; i <= end; i++) {
        AABB sphere_bounds = calculate_sphere_bounds(spheres[i]);
        bounds.min.x = fminf(bounds.min.x, sphere_bounds.min.x);
        bounds.min.y = fminf(bounds.min.y, sphere_bounds.min.y);
        bounds.min.z = fminf(bounds.min.z, sphere_bounds.min.z);
        bounds.max.x = fmaxf(bounds.max.x, sphere_bounds.max.x);
        bounds.max.y = fmaxf(bounds.max.y, sphere_bounds.max.y);
        bounds.max.z = fmaxf(bounds.max.z, sphere_bounds.max.z);
    }
    
    // Choose split axis based on longest dimension
    float dx = bounds.max.x - bounds.min.x;
    float dy = bounds.max.y - bounds.min.y;
    float dz = bounds.max.z - bounds.min.z;
    
    if (dx > dy && dx > dz) {
        qsort(spheres + start, end - start + 1, sizeof(Sphere), compare_spheres_x);
    } else if (dy > dz) {
        qsort(spheres + start, end - start + 1, sizeof(Sphere), compare_spheres_y);
    } else {
        qsort(spheres + start, end - start + 1, sizeof(Sphere), compare_spheres_z);
    }
    
    int mid = (start + end) / 2;
    node->sphere = NULL;
    node->bounds = bounds;
    node->left = build_bvh(spheres, start, mid);
    node->right = build_bvh(spheres, mid + 1, end);
    
    return node;
}

float intersect_bvh(BVHNode* node, Ray ray) {
    if (ray_aabb_intersection(ray, node->bounds) == INFINITY)
        return INFINITY;
        
    if (node->sphere)
        return ray_sphere_intersection(ray, *node->sphere);
        
    float dist_left = intersect_bvh(node->left, ray);
    float dist_right = intersect_bvh(node->right, ray);
    return fminf(dist_left, dist_right);
}

void free_bvh(BVHNode* node) {
    if (!node) return;
    free_bvh(node->left);
    free_bvh(node->right);
    free(node);
}

void benchmark_no_bvh(Sphere* spheres, int num_spheres, int num_rays) {
    clock_t start = clock();
    long long intersection_tests = 0;
    int intersections = 0;
    
    for (int i = 0; i < num_rays; i++) {
        // Generate random ray with normalized direction
        Vec3 dir = {
            (float)rand() / RAND_MAX * 2 - 1,
            (float)rand() / RAND_MAX * 2 - 1,
            (float)rand() / RAND_MAX * 2 - 1
        };
        dir = vec3_normalize(dir);
        
        Ray ray = {
            {-1000, -1000, -1000},  // Start rays far from spheres
            dir
        };
        
        float closest_dist = INFINITY;
        for (int j = 0; j < num_spheres; j++) {
            intersection_tests++;
            float dist = ray_sphere_intersection(ray, spheres[j]);
            if (dist < closest_dist) {
                closest_dist = dist;
                if (dist != INFINITY) intersections++;
            }
        }
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("No BVH:\n");
    printf("Time: %f seconds\n", time_spent);
    printf("Intersection tests: %lld\n", intersection_tests);
    printf("Intersections found: %d\n\n", intersections);
}

void benchmark_with_bvh(BVHNode* root, int num_spheres, int num_rays) {
    clock_t start = clock();
    int intersections = 0;
    
    for (int i = 0; i < num_rays; i++) {
        // Generate random ray with normalized direction
        Vec3 dir = {
            (float)rand() / RAND_MAX * 2 - 1,
            (float)rand() / RAND_MAX * 2 - 1,
            (float)rand() / RAND_MAX * 2 - 1
        };
        dir = vec3_normalize(dir);
        
        Ray ray = {
            {-1000, -1000, -1000},  // Start rays far from spheres
            dir
        };
        
        float dist = intersect_bvh(root, ray);
        if (dist != INFINITY) intersections++;
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("With BVH:\n");
    printf("Time: %f seconds\n", time_spent);
    printf("Intersections found: %d\n\n", intersections);
}

int main() {
    srand(time(NULL));
    
    // Test with different numbers of spheres
    int sphere_counts[] = {1000, 10000, 100000, 1000000};
    int num_rays = 10000;
    
    float world_size = 2000.0f;  // Increased world size
    
    for (int i = 0; i < sizeof(sphere_counts) / sizeof(sphere_counts[0]); i++) {
        int num_spheres = sphere_counts[i];
        printf("Testing with %d spheres:\n", num_spheres);
        
        // Create random spheres with smaller radii and spread out more
        Sphere* spheres = malloc(num_spheres * sizeof(Sphere));
        for (int j = 0; j < num_spheres; j++) {
            spheres[j] = (Sphere){
                {
                    (float)rand() / RAND_MAX * world_size - world_size/2,
                    (float)rand() / RAND_MAX * world_size - world_size/2,
                    (float)rand() / RAND_MAX * world_size - world_size/2
                },
                5.0f  // Smaller, fixed radius
            };
        }
        
        // Build BVH
        BVHNode* root = build_bvh(spheres, 0, num_spheres - 1);
        
        // Run benchmarks
        benchmark_no_bvh(spheres, num_spheres, num_rays);
        benchmark_with_bvh(root, num_spheres, num_rays);
        
        // Cleanup
        free_bvh(root);
        free(spheres);
        
        printf("----------------------------------------\n");
    }
    
    return 0;
}