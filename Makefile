CC = clang
CXX = clang++

CFLAGS = -std=c11 \
				 -I/opt/homebrew/Cellar/glfw/3.4/include \
				 -I/opt/homebrew/Cellar/cglm/0.9.6/include \
				 -I/Users/tristanlowe/VulkanSDK/1.4.313.1/macOS/include

CXXFLAGS = -std=c++17 \
					 -I/opt/homebrew/Cellar/glfw/3.4/include \
					 -I/opt/homebrew/Cellar/cglm/0.9.6/include \
					 -I/Users/tristanlowe/VulkanSDK/1.4.313.1/macOS/include \
					 -Wno-nullability-completeness

LDFLAGS = \
					-L/opt/homebrew/Cellar/glfw/3.4/lib -lglfw \
					-L/Users/tristanlowe/VulkanSDK/1.4.313.1/macOS/lib -lvulkan \
					-Wl,-rpath,/Users/tristanlowe/VulkanSDK/1.4.313.1/macOS/lib \
					-framework Metal -framework QuartzCore

SRC_C = $(wildcard src/*.c)
OBJ_C = $(SRC_C:src/%.c=build/%.o)

OBJ = $(OBJ_C) build/vma_wrapper.o
BIN = build/app

#######################
### Default Build ###
#######################

.PHONY: all
all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

#########################
### VMA Wrapper Only ###
#########################

.PHONY: vma
vma:
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c src/vma_wrapper.cpp -o build/vma_wrapper.o

#######################
### Shader Compile ###
#######################

SHADER_VERT = src/shaders/shader.vert
SHADER_FRAG = src/shaders/shader.frag
SHADER_VERT_SPV = src/shaders/vert.spv
SHADER_FRAG_SPV = src/shaders/frag.spv

.PHONY: shaders
shaders: $(SHADER_VERT_SPV) $(SHADER_FRAG_SPV)

$(SHADER_VERT_SPV): $(SHADER_VERT)
	glslangValidator -V $< -o $@

$(SHADER_FRAG_SPV): $(SHADER_FRAG)
	glslangValidator -V $< -o $@

#######################
### Clean ###
#######################

.PHONY: clean
clean:
	rm -rf build
	rm -f src/shaders/*.spv
