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

LBR_OBJ = build/libraries.o
OBJ = $(OBJ_C) $(LBR_OBJ)

BIN = build/app

#########################
### Default Build All ###
#########################

.PHONY: all
all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

##########################
### Fast Relink Only ###
##########################

.PHONY: fast
fast: $(BIN)

#########################
### VMA Wrapper Only ###
#########################

.PHONY: lbr
lbr: $(LBR_OBJ)

$(LBR_OBJ): src/libraries.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

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
### Clean Targets ###
#######################

.PHONY: clean
clean:
	rm -f $(BIN)

.PHONY: clean\ vma
clean\ vma:
	rm -f $(LBR_OBJ)

.PHONY: clean\ shaders
clean\ shaders:
	rm -f $(SHADER_VERT_SPV) $(SHADER_FRAG_SPV)
