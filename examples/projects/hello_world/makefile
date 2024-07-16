
# config
INCLUDE_PATHS := \
	-I.

BUILD_FLAGS := \
	-a z80 \
	-f elf64 \
	-e _start

SOURCE_FILE = ./main.lasm
OUTPUT_NAME = hello_world

# -------------------------------------------------- #
# do not change anything below this line (unless you
# know what you are doing).

# commands
CPP = cpp
LASM = ./../../../build/lasm

# flags
CPP_FLAGS = -E $(INCLUDE_PATHS)
LASM_FLAGS = $(BUILD_FLAGS)

# directories
SOURCE_DIR = ./
BUILD_DIR = ./build

# output files
OUTPUT_INT = $(BUILD_DIR)/$(OUTPUT_NAME).int
OUTPUT_BIN = $(BUILD_DIR)/$(OUTPUT_NAME).bin

# targets
all: setup build

setup:
	mkdir -p $(BUILD_DIR)
	echo $(shell pwd) > $(BUILD_DIR)/project_root.txt

build: $(OUTPUT_BIN)

$(OUTPUT_BIN): $(OUTPUT_INT)
	$(LASM) build $(LASM_FLAGS) -o $(OUTPUT_BIN) $(OUTPUT_INT)

$(OUTPUT_INT): $(SOURCE_DIR)/$(SOURCE_FILE)
	$(CPP) $(CPP_FLAGS) -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all setup build clean

# -------------------------------------------------- #
