#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Vector and basic structures
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

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Ray-Sphere intersection
int ray_sphere_intersection(Ray ray, Sphere sphere) {
    Vec3 oc = vec3_sub(ray.origin, sphere.center);
    float a = vec3_dot(ray.direction, ray.direction);
    float b = 2.0f * vec3_dot(oc, ray.direction);
    float c = vec3_dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4 * a * c;
    return discriminant > 0;
}

// AABB intersection
int ray_aabb_intersection(Ray ray, AABB bounds) {
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

    return tmax >= tmin && tmax > 0;
}

// BVH functions
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

BVHNode* build_bvh(Sphere* spheres, int start, int end) {
    BVHNode* node = malloc(sizeof(BVHNode));
    
    if (start == end) {
        node->sphere = &spheres[start];
        node->bounds = calculate_sphere_bounds(spheres[start]);
        node->left = node->right = NULL;
        return node;
    }
    
    int mid = (start + end) / 2;
    node->sphere = NULL;
    node->left = build_bvh(spheres, start, mid);
    node->right = build_bvh(spheres, mid + 1, end);
    
    // Calculate bounds for internal node
    node->bounds.min = (Vec3){
        fminf(node->left->bounds.min.x, node->right->bounds.min.x),
        fminf(node->left->bounds.min.y, node->right->bounds.min.y),
        fminf(node->left->bounds.min.z, node->right->bounds.min.z)
    };
    node->bounds.max = (Vec3){
        fmaxf(node->left->bounds.max.x, node->right->bounds.max.x),
        fmaxf(node->left->bounds.max.y, node->right->bounds.max.y),
        fmaxf(node->left->bounds.max.z, node->right->bounds.max.z)
    };
    
    return node;
}

int intersect_bvh(BVHNode* node, Ray ray) {
    if (!ray_aabb_intersection(ray, node->bounds))
        return 0;
        
    if (node->sphere)
        return ray_sphere_intersection(ray, *node->sphere);
        
    return intersect_bvh(node->left, ray) || 
           intersect_bvh(node->right, ray);
}

void free_bvh(BVHNode* node) {
    if (!node) return;
    free_bvh(node->left);
    free_bvh(node->right);
    free(node);
}

// Benchmark functions
void benchmark_no_bvh(Sphere* spheres, int num_spheres, int num_rays) {
    clock_t start = clock();
    int intersections = 0;
    
    for (int i = 0; i < num_rays; i++) {
        // Generate random ray
        Ray ray = {
            {rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50},
            {rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50}
        };
        
        // Test against all spheres
        for (int j = 0; j < num_spheres; j++) {
            if (ray_sphere_intersection(ray, spheres[j])) {
                intersections++;
                break;  // Count first intersection only
            }
        }
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("No BVH:\n");
    printf("Time: %f seconds\n", time_spent);
    printf("Rays: %d\n", num_rays);
    printf("Spheres: %d\n", num_spheres);
    printf("Intersections found: %d\n\n", intersections);
}

void benchmark_with_bvh(BVHNode* root, int num_spheres, int num_rays) {
    clock_t start = clock();
    int intersections = 0;
    
    for (int i = 0; i < num_rays; i++) {
        // Generate random ray
        Ray ray = {
            {rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50},
            {rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50}
        };
        
        if (intersect_bvh(root, ray)) {
            intersections++;
        }
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("With BVH:\n");
    printf("Time: %f seconds\n", time_spent);
    printf("Rays: %d\n", num_rays);
    printf("Spheres: %d\n", num_spheres);
    printf("Intersections found: %d\n\n", intersections);
}

int main() {
    srand(time(NULL));
    
    // Test with different numbers of spheres
    int sphere_counts[] = {100, 1000, 10000, 100000, 1000000};
    int num_rays = 10000;  // Number of random rays to test
    
    for (int i = 0; i < sizeof(sphere_counts) / sizeof(sphere_counts[0]); i++) {
        int num_spheres = sphere_counts[i];
        printf("Testing with %d spheres:\n", num_spheres);
        
        // Create random spheres
        Sphere* spheres = malloc(num_spheres * sizeof(Sphere));
        for (int j = 0; j < num_spheres; j++) {
            spheres[j] = (Sphere){
                {rand() % 100 - 50, rand() % 100 - 50, rand() % 100 - 50},
                1.0f + (rand() % 5)
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