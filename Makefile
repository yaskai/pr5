INC_DIR := include

# Compiler and flags
CC := g++
#CFLAGS := -Wall -O2 -Ibuild/external/raylib/src -DPLATFORM_DESKTOP_SDL
CFLAGS := -g -Wall -O2 -Ibuild/external/raylib/src -I/usr/include/SDL3 -DPLATFORM_DESKTOP_SDL -std=c++17
LDFLAGS := -lSDL3 -lm -ldl -lpthread -lGL -lrt -lX11

#CFLAGS := -g -Wall -std=c99 -O2 -I$(INC_DIR) -Ibuild/external/raylib/src -DPLATFORM_DESKTOP
#LDFLAGS := -lm -ldl -lpthread -lGL -lrt -lX11

#PLATFORM=PLATFORM_DESKTOP_SDL3 SDL_INCLUDE_PATH=/usr/include/SDL3 SDL_LIBRARY_PATH=/usr/lib SDL_LIBRARIES="-lSDL3" GRAPHICS=GRAPHICS_API_OPENG_33

# Paths
SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin
RAYLIB_DIR := build/external/raylib
RAYLIB_LIB := $(RAYLIB_DIR)/src/libraylib.a

# Sources and objects
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
SRCS += $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o,$(SRCS))

# Output executable
TARGET := $(BIN_DIR)/game

.PHONY: all clean directories

all: directories $(TARGET)

# Link, using explicit path to libraylib.a (no -lraylib)
$(TARGET): $(OBJS) 
	$(CC) $^ -o $@ $(RAYLIB_LIB) $(LDFLAGS)

# Compile .c to .o in build/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | directories
	$(CC) $(CFLAGS) -c $< -o $@

# Create build and bin dirs if missing
directories:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/*

