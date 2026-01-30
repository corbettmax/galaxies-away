# Galaxies Away - Space Roguelike Shoot'em Up
# Makefile for Linux/macOS

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
DEBUG_FLAGS := -g -DDEBUG

# Directories
SRC_DIR := src
BUILD_DIR := build
SHADER_DIR := shaders
ASSETS_DIR := assets

# Source files
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPENDS := $(OBJECTS:.o=.d)

# Output executable
TARGET := galaxies_away

# Libraries
# Detect OS
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    LIBS := -lGL -lglfw -ldl -lpthread -lm -lfreetype
    INCLUDES := -I/usr/include -I/usr/include/freetype2
endif

ifeq ($(UNAME_S),Darwin)
    LIBS := -framework OpenGL -lglfw -ldl -lpthread -lfreetype
    INCLUDES := -I/usr/local/include -I/opt/homebrew/include -I/usr/local/include/freetype2 -I/opt/homebrew/include/freetype2
    LDFLAGS := -L/usr/local/lib -L/opt/homebrew/lib
endif

# Windows (MinGW) - uncomment if needed
# LIBS := -lopengl32 -lglfw3 -lgdi32
# INCLUDES := -IC:/path/to/includes

# Default target
all: directories $(TARGET)

# Create build directory
directories:
	@mkdir -p $(BUILD_DIR)

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "Build complete: $(TARGET)"

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Include dependencies
-include $(DEPENDS)

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean all

# Run the game
run: all
	./$(TARGET)

# Clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned build files"

# Install dependencies (Linux - Debian/Ubuntu)
install-deps-linux:
	sudo apt-get update
	sudo apt-get install -y libglfw3-dev libglm-dev libfreetype6-dev build-essential

# Install dependencies (macOS)
install-deps-macos:
	brew install glfw glm freetype

# Help
help:
	@echo "Galaxies Away - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build the game (default)"
	@echo "  debug            - Build with debug symbols"
	@echo "  run              - Build and run the game"
	@echo "  clean            - Remove build files"
	@echo "  install-deps-linux  - Install dependencies on Debian/Ubuntu"
	@echo "  install-deps-macos  - Install dependencies on macOS"
	@echo "  help             - Show this help message"

.PHONY: all directories debug run clean install-deps-linux install-deps-macos help
