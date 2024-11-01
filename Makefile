# Compiler and flags
CC := gcc
CFLAGS := -Wall -Iinclude 

# Source and target
SRC := src/vec3.c src/camera.c src/sphere.c src/ray.c src/bvh.c src/hit.c src/renderer.c src/main.c src/benchmark.c
TARGET := raytracer

# Check for gnuplot
GNUPLOT := $(shell which gnuplot)
ifeq ($(GNUPLOT),)
    $(error "gnuplot not found. Please install gnuplot")
endif

CFLAGS += -DGNUPLOT_PATH=\"$(GNUPLOT)\"

# SDL2 library and includes setup
ifeq ($(OS), Windows_NT)
    SDL_INCLUDE := -Iinclude
    # Make sure we link mingw32 first, then SDL2main, then SDL2
    SDL_LIB := -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
    TARGET := $(TARGET).exe
    RM := del /Q
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        # macOS specific settings
        SDL_INCLUDE := $(shell sdl2-config --cflags)
        SDL_LIB := $(shell sdl2-config --libs) -lSDL2_image
        CFLAGS += -arch arm64  # For Apple Silicon
    else
        # Linux settings
        SDL_INCLUDE := $(shell sdl2-config --cflags)
        SDL_LIB := $(shell sdl2-config --libs)
    endif
    RM := rm -f
endif

# Compile with debug information
CFLAGS += -g

# Compile
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SDL_INCLUDE) $(SRC) -o $(TARGET) $(SDL_LIB)

# Clean up build artifacts
clean:
	$(RM) $(TARGET)


