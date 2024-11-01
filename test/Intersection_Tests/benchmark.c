#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "Custom/benchmark.h"
#include "Custom/hit.h"



void benchmark_no_bvh(Sphere* spheres, int num_spheres, int num_rays) {
    clock_t start = clock();
    long long intersection_tests = 0;
    int intersections = 0;
    
    for (int i = 0; i < num_rays; i++) {
        Vec3 dir = {
            (float)rand() / RAND_MAX * 2 - 1,
            (float)rand() / RAND_MAX * 2 - 1,
            (float)rand() / RAND_MAX * 2 - 1
        };
        dir = vec3_normalize(dir);
        
        Ray ray = {
            {-1000, -1000, -1000},
            dir
        };
        
        float closest_dist = INFINITY;
        for (int j = 0; j < num_spheres; j++) {
            intersection_tests++;
            HitRecord hit = ray_sphere_intersect(ray, &spheres[j]);
            if (hit.t < closest_dist) {
                closest_dist = hit.t;
                if (hit.t != INFINITY) intersections++;
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
        Vec3 dir = {
            (float)rand() / RAND_MAX * 2 - 1,
            (float)rand() / RAND_MAX * 2 - 1,
            (float)rand() / RAND_MAX * 2 - 1
        };
        dir = vec3_normalize(dir);
        
        Ray ray = {
            {-1000, -1000, -1000},
            dir
        };
        
        HitRecord hit = ray_bvh_intersect(ray, root);
        if (hit.t != INFINITY) intersections++;
    }
    
    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("With BVH:\n");
    printf("Time: %f seconds\n", time_spent);
    printf("Intersections found: %d\n\n", intersections);
}