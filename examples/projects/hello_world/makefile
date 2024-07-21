
# ------------------------------------------------------------------ #
# config

include_paths := -I. -I./../../../stdlib

lasm_flags := -a rl78 -f elf64 -e _start

source_file := main.lasm
output_name := hello_world

lasm := ./../../../build/lasm
# ------------------------------------------------------------------ #
# note: do not change anything below this line (unless you know what
# you are doing).

# flags
cpp_flags = -E $(include_paths)

# directories
source_dir_path = ./
build_dir_path = ./build

# output files
output_int_file = $(build_dir_path)/$(output_name).int
output_bin_file = $(build_dir_path)/$(output_name).bin

# targets
all: setup build

setup:
	mkdir -p $(build_dir_path)
	echo $(shell pwd) > $(build_dir_path)/project_root.txt

build: $(output_bin_file)

$(output_bin_file): $(output_int_file)
	$(lasm) build $(lasm_flags) -o $(output_bin_file) $(output_int_file)

$(output_int_file): $(source_dir_path)/$(source_file)
	cpp $(cpp_flags) -o $@ $<

clean:
	rm -rf $(build_dir_path)

.PHONY: all setup build clean
# ------------------------------------------------------------------ #
