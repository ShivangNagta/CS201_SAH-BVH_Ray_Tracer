# Compiler and flags
CC := gcc
CFLAGS := -Wall -Iinclude

# Source and target
SRC := src/main.c
TARGET := raytracer

# SDL2 library and includes setup
ifeq ($(OS), Windows_NT)
    SDL_INCLUDE := -Iinclude
    # Make sure we link mingw32 first, then SDL2main, then SDL2
    SDL_LIB := -Llib -lmingw32 -lSDL2main -lSDL2
    TARGET := $(TARGET).exe
    RM := del /Q
else
    SDL_INCLUDE := -I/usr/local/include/SDL2
    SDL_LIB := -L/usr/local/lib -lSDL2main -lSDL2
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