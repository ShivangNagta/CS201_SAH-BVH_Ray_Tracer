#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>
#include "Custom/benchmark.h"
#include "Custom/hit.h"

#ifndef GNUPLOT_PATH
#define GNUPLOT_PATH "gnuplot"
#endif

// In benchmark.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #define PRIu64 "I64u"
    #include <windows.h>
#else
    #include <inttypes.h>
#endif

void run_gnuplot(void) {
    #ifdef _WIN32
        char command[512];
        // Convert GNUPLOT_PATH to use forward slashes
        char gnuplot_path[256];
        strncpy(gnuplot_path, GNUPLOT_PATH, sizeof(gnuplot_path) - 1);
        for (char *p = gnuplot_path; *p; p++) {
            if (*p == '\\') *p = '/';
        }
        
        // Create the command with proper quoting
        snprintf(command, sizeof(command), 
                "cmd /c \"\"%s\" \"%s/plot_benchmark.gnu\"\"", 
                gnuplot_path,
                getcwd(NULL, 0));
    #else
        char command[256];
        snprintf(command, sizeof(command), 
                "%s plot_benchmark.gnu", 
                GNUPLOT_PATH);
    #endif

    printf("Executing command: %s\n", command);
    int result = system(command);
    
    if (result != 0) {
        printf("Warning: Gnuplot command returned error code %d\n", result);
    }

    // Wait a moment for the file to be created
    #ifdef _WIN32
        Sleep(1000);  // Wait 1 second
    #else
        sleep(1);
    #endif

    // Check if the output file exists
    FILE* test = fopen("benchmark_results.png", "rb");
    if (test) {
        fclose(test);
        printf("Successfully created benchmark_results.png\n");
    } else {
        printf("Warning: benchmark_results.png was not created\n");
    }
}


void free_bvh(BVHNode* node) {
    if (!node) return;
    free_bvh(node->left);
    free_bvh(node->right);
    free(node);
}

void create_gnuplot_script(const char* data_filename) {
    FILE* gnuplot_script = fopen("plot_benchmark.gnu", "w");
    if (gnuplot_script == NULL) {
        printf("Error creating gnuplot script!\n");
        return;
    }

    fprintf(gnuplot_script, "set terminal png size 800,600\n");
    fprintf(gnuplot_script, "set output 'benchmark_results.png'\n");
    fprintf(gnuplot_script, "set title 'Ray Tracing Performance: BVH vs No BVH'\n");
    fprintf(gnuplot_script, "set xlabel 'Number of Spheres'\n");
    fprintf(gnuplot_script, "set ylabel 'Time (seconds)'\n");
    
    // Remove logarithmic scaling
    // fprintf(gnuplot_script, "set logscale x\n");
    // fprintf(gnuplot_script, "set logscale y\n");
    
    // Set explicit tick marks for x-axis
    fprintf(gnuplot_script, "set xtics (");
    // fprintf(gnuplot_script, "'1K' 1000, '10K' 10000, '100K' 100000, '1M' 1000000");
    fprintf(gnuplot_script, "'1K' 1000, '10K' 10000");
    fprintf(gnuplot_script, ")\n");
    
    // Enable grid
    fprintf(gnuplot_script, "set grid xtics ytics\n");
    
    // Set the same distance between tick marks
    fprintf(gnuplot_script, "set xtics scale 1\n");
    fprintf(gnuplot_script, "set ytics scale 1\n");
    
    // Force equal spacing between tick marks
    fprintf(gnuplot_script, "set xtics rotate by -45\n");  // Rotate labels for better readability
    
    // Set key (legend) position
    fprintf(gnuplot_script, "set key top left\n");
    
    // Use line styles with points
    fprintf(gnuplot_script, "set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 7 ps 1.5\n");  // Blue
    fprintf(gnuplot_script, "set style line 2 lc rgb '#dd181f' lt 1 lw 2 pt 7 ps 1.5\n");  // Red
    
    // Plot the data with custom styles
    fprintf(gnuplot_script, "plot '%s' using 1:2 with linespoints ls 1 title 'No BVH', \\\n", data_filename);
    fprintf(gnuplot_script, "     '%s' using 1:3 with linespoints ls 2 title 'With BVH'\n", data_filename);
    
    fclose(gnuplot_script);
}

// Helper function to get max time for y-axis scaling
double get_max_time(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0.0;
    
    double max_time = 0.0;
    int spheres;
    double time1, time2;
    
    while (fscanf(file, "%d %lf %lf", &spheres, &time1, &time2) == 3) {
        if (time1 > max_time) max_time = time1;
        if (time2 > max_time) max_time = time2;
    }
    
    fclose(file);
    return max_time;
}








// Function to save benchmark results to a file
void save_benchmark_data(const char* filename, int sphere_count, double time_no_bvh, double time_with_bvh) {
    FILE* file = fopen(filename, "a");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }
    fprintf(file, "%d %f %f\n", sphere_count, time_no_bvh, time_with_bvh);
    fclose(file);
}


double benchmark_no_bvh(Sphere* spheres, int num_spheres, int num_rays) {
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
#ifdef _WIN32
    printf("Intersection tests: %llu\n", intersection_tests);
#else
    printf("Intersection tests: %lld\n", intersection_tests);
#endif
    printf("Intersections found: %d\n\n", intersections);
    
    return time_spent;
}

double benchmark_with_bvh(BVHNode* root, int num_spheres, int num_rays) {
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
    
    return time_spent;
}

void run_benchmark_with_plotting() {
    // Remove existing data file
    remove("benchmark_data.txt");
    
    srand(time(NULL));
    
    // Use equally spaced points
     int sphere_counts[10];
     int s = 50;
    for(int i = 0; i < 10; i++){
        sphere_counts[i] = s;
        s+= 50;
    }
    // int sphere_counts[] = {1000, 10000, 100000, 1000000};  // 1K to 1M
    int num_rays = 10000;
    float world_size = 2000.0f;
    
    for (int i = 0; i < sizeof(sphere_counts) / sizeof(sphere_counts[0]); i++) {
        int num_spheres = sphere_counts[i];
        printf("Testing with %d spheres:\n", num_spheres);
        
        Sphere* spheres = malloc(num_spheres * sizeof(Sphere));
        for (int j = 0; j < num_spheres; j++) {
            Vec3 center = {
                (float)rand() / RAND_MAX * world_size - world_size/2,
                (float)rand() / RAND_MAX * world_size - world_size/2,
                (float)rand() / RAND_MAX * world_size - world_size/2
            };
            spheres[j] = create_benchmark_sphere(center);
        }
        
        BVHNode* root = build_bvh_node(spheres, 0, num_spheres - 1, 20);
        
        double time_no_bvh = benchmark_no_bvh(spheres, num_spheres, num_rays);
        double time_with_bvh = benchmark_with_bvh(root, num_spheres, num_rays);
        
        // Save results to file
        save_benchmark_data("benchmark_data.txt", num_spheres, time_no_bvh, time_with_bvh);
        
        free_bvh(root);
        free(spheres);
        
        printf("----------------------------------------\n");
    }
    
    // Create and execute gnuplot script
    create_gnuplot_script("benchmark_data.txt");
    run_gnuplot();
    printf("\nBenchmark plot has been saved as 'benchmark_results.png'\n");
}