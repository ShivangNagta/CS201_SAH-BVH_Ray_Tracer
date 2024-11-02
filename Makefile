# Compiler and flags
CC := gcc
CFLAGS := -Wall -Iinclude 

# Source and target
SRC := src/vec3.c src/camera.c src/sphere.c src/ray.c src/bvh.c src/hit.c src/renderer.c src/main.c src/benchmark.c src/bvh_visualiser.c
TARGET := raytracer

# OS-specific settings
ifeq ($(OS),Windows_NT)
    # Windows-specific settings
    GNUPLOT_PATH := C:/PROGRA~1/gnuplot/bin/gnuplot.exe
    
   
    ifeq ($(wildcard $(GNUPLOT_PATH)),)
        GNUPLOT_PATH := C:/Program\ Files/gnuplot/bin/gnuplot.exe
    endif
    
    SDL_INCLUDE := -Iinclude
    SDL_LIB := -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
    TARGET := $(TARGET).exe
    RM := del /Q
else
    # Unix-like systems (Linux/macOS)
    UNAME_S := $(shell uname -s)
    GNUPLOT_PATH := $(shell which gnuplot)
    
    ifeq ($(UNAME_S),Darwin)
        SDL_INCLUDE := $(shell sdl2-config --cflags)
        SDL_LIB := $(shell sdl2-config --libs) -lSDL2_image
        CFLAGS += -arch arm64
    else
        SDL_INCLUDE := $(shell sdl2-config --cflags)
        SDL_LIB := $(shell sdl2-config --libs)
    endif
    RM := rm -f
endif

# Define GNUPLOT_PATH
CFLAGS += -DGNUPLOT_PATH=\"$(GNUPLOT_PATH)\"

# Compile with debug information
CFLAGS += -g

# OpenMP support
# CFLAGS += -fopenmp
# LDFLAGS += -fopenmp

# Compile
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SDL_INCLUDE) $(SRC) -o $(TARGET) $(SDL_LIB) $(LDFLAGS)

clean:
	$(RM) $(TARGET)
	$(RM) benchmark_results.png
	$(RM) benchmark_data.txt

.PHONY: all clean