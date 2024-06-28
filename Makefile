CC := clang++

BLACK        := $(shell tput -Txterm setaf 0)
RED          := $(shell tput -Txterm setaf 1)
GREEN        := $(shell tput -Txterm setaf 2)
YELLOW       := $(shell tput -Txterm setaf 3)
LIGHTPURPLE  := $(shell tput -Txterm setaf 4)
PURPLE       := $(shell tput -Txterm setaf 5)
BLUE         := $(shell tput -Txterm setaf 6)
WHITE        := $(shell tput -Txterm setaf 7)
RESET        := $(shell tput -Txterm sgr0)

COMMONFLAGS := -g -ggdb -Wall -fms-extensions

CFLAGS := $(COMMONFLAGS) $(shell pkg-config --cflags sdl2)

LDFLAGS := $(COMMONFLAGS) -lvulkan $(shell pkg-config --libs sdl2)

SOURCES_DIR := src
BUILD_DIR := build
SHADERS_DIR := shaders

SOURCES := $(wildcard $(SOURCES_DIR)/**/**/**/*.cpp $(SOURCES_DIR)/**/**/*.cpp $(SOURCES_DIR)/**/*.cpp $(SOURCES_DIR)/*.cpp)
HEADERS := $(wildcard $(SOURCES_DIR)/**/**/**/*.hpp $(SOURCES_DIR)/**/**/*.hpp $(SOURCES_DIR)/**/*.hpp $(SOURCES_DIR)/*.hpp)

SHADERS := $(wildcard $(SHADERS_DIR)/*.frag)
SHADERS += $(wildcard $(SHADERS_DIR)/*.vert)

SHADERS_SPV := $(SHADERS:.frag=.frag.spv)
SHADERS_SPV := $(SHADERS_SPV:.vert=.vert.spv)

OBJECTS := $(patsubst $(SOURCES_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

TARGET := render

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(info $(RESET)[$(GREEN)LINK$(RESET)] $^ -> $@)
	@$(CC) $(LDFLAGS) -o $@ $^ 

$(BUILD_DIR)/%.o: $(SOURCES_DIR)/%.cpp $(HEADERS)
	$(info $(RESET)[$(GREEN)CC$(RESET)] $< -> $@)
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(SHADERS_DIR)/%.frag.spv: $(SHADERS_DIR)/%.frag
	glslangValidator -V $< -o $@

$(SHADERS_DIR)/%.vert.spv: $(SHADERS_DIR)/%.vert
	glslangValidator -V $< -o $@

clean:
	rm -frv $(BUILD_DIR)/**

debug:
	gdb $(BUILD_DIR)/$(TARGET)

run:
	$(BUILD_DIR)/$(TARGET)

shaders: $(SHADERS_SPV)

.PHONY: clean debug run

all: $(BUILD_DIR)/$(TARGET) $(SHADERS_SPV)