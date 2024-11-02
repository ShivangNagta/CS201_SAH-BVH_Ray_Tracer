#pragma once

#include "Custom/sphere.h"
#include "Custom/bvh.h"


void free_bvh(BVHNode* node);
void create_gnuplot_script(const char* data_filename);
void run_gnuplot();
void save_benchmark_data(const char* filename, int sphere_count, double time_no_bvh, double time_with_bvh);
void create_gnuplot_script(const char* data_filename);
double benchmark_no_bvh(Sphere* spheres, int num_spheres, int num_rays);
double benchmark_with_bvh(BVHNode* root, int num_spheres, int num_rays);
void print_sphere_info(Sphere *spheres, int num_spheres);
void run_benchmark_with_plotting();