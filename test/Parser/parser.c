
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    int v1, v2, v3;
} Face;

typedef struct {
    Vertex *vertices;
    Face *faces;
    int vertex_count;
    int face_count;
} Model;

Model load_obj(const char *filename) {
    Model model = {NULL, NULL, 0, 0};
    FILE *file = fopen(filename, "r");

    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return model;
    }

    // First pass to count vertices and faces
    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            model.vertex_count++;
        } else if (line[0] == 'f' && line[1] == ' ') {
            model.face_count++;
        }
    }

    // Allocate memory
    model.vertices = malloc(sizeof(Vertex) * model.vertex_count);
    model.faces = malloc(sizeof(Face) * model.face_count);

    // Second pass to load vertices and faces
    rewind(file);
    int vertex_index = 0;
    int face_index = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            sscanf(line, "v %f %f %f", &model.vertices[vertex_index].x, &model.vertices[vertex_index].y, &model.vertices[vertex_index].z);
            vertex_index++;
        } else if (line[0] == 'f' && line[1] == ' ') {
            sscanf(line, "f %d %d %d", &model.faces[face_index].v1, &model.faces[face_index].v2, &model.faces[face_index].v3);
            face_index++;
        }
    }

    fclose(file);
    return model;
}

void free_model(Model *model) {
    free(model->vertices);
    free(model->faces);
    model->vertex_count = 0;
    model->face_count = 0;
}

void render_model(SDL_Renderer *renderer, const Model *model, int width, int height) {
    for (int i = 0; i < model->face_count; i++) {
        Face face = model->faces[i];

        // Retrieve vertices of the face (adjust 1-based OBJ index to 0-based array index)
        Vertex v1 = model->vertices[face.v1 - 1];
        Vertex v2 = model->vertices[face.v2 - 1];
        Vertex v3 = model->vertices[face.v3 - 1];

        // Simple orthographic projection to convert 3D to 2D
        int x1 = (int)(width / 2 + v1.x * 1000);
        int y1 = (int)(height / 2 - v1.y * 1000);
        int x2 = (int)(width / 2 + v2.x * 1000);
        int y2 = (int)(height / 2 - v2.y * 1000);
        int x3 = (int)(width / 2 + v3.x * 1000);
        int y3 = (int)(height / 2 - v3.y * 1000);

        // Draw edges of the triangle
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        SDL_RenderDrawLine(renderer, x2, y2, x3, y3);
        SDL_RenderDrawLine(renderer, x3, y3, x1, y1);
    }
}

int main() {
    const char *filename = "./bunny.obj";  // Path to the OBJ file

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("OBJ Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Model model = load_obj(filename);
    if (model.vertex_count == 0 || model.face_count == 0) {
        fprintf(stderr, "Failed to load OBJ model\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        // Set color for model
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        render_model(renderer, &model, 800, 600);

        SDL_RenderPresent(renderer);
    }

    free_model(&model);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
