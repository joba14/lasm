
#define lasm_template_makefile_fmt                                             \
	"\n"                                                                       \
	"# ------------------------------------------------------------------ #\n" \
	"# config\n"                                                               \
	"include_paths := -I.\n"                                                   \
	"\n"                                                                       \
	"lasm_flags := -a <lasm_arch> -f <lasm_format>\n"                          \
	"\n"                                                                       \
	"source_file := entry.lasm\n"                                              \
	"output_name := entry\n"                                                   \
	"\n"                                                                       \
	"lasm := <set_the_lasm_path_here>\n"                                       \
	"# ------------------------------------------------------------------ #\n" \
	"# note: do not change anything below this line (unless you know what\n"   \
	"# you are doing).\n"                                                      \
	"\n"                                                                       \
	"# flags\n"                                                                \
	"cpp_flags = -E $(include_paths)\n"                                        \
	"\n"                                                                       \
	"# directories\n"                                                          \
	"source_dir_path = ./\n"                                                   \
	"build_dir_path = ./build\n"                                               \
	"\n"                                                                       \
	"# output files\n"                                                         \
	"output_int_file = $(build_dir_path)/$(output_name).int\n"                 \
	"output_bin_file = $(build_dir_path)/$(output_name).bin\n"                 \
	"\n"                                                                       \
	"# targets\n"                                                              \
	"all: setup build\n"                                                       \
	"\n"                                                                       \
	"setup:\n"                                                                 \
	"	mkdir -p $(build_dir_path)\n"                                          \
	"	echo $(shell pwd) > $(build_dir_path)/project_root.txt\n"              \
	"\n"                                                                       \
	"build: $(output_bin_file)\n"                                              \
	"\n"                                                                       \
	"$(output_bin_file): $(output_int_file)\n"                                 \
	"	$(lasm) build $(lasm_flags) -o $(output_bin_file) $(output_int_file)\n"\
	"\n"                                                                       \
	"$(output_int_file): $(source_dir_path)/$(source_file)\n"                  \
	"	cpp $(cpp_flags) -o $@ $<\n"                                           \
	"\n"                                                                       \
	"clean:\n"                                                                 \
	"	rm -rf $(build_dir_path)\n"                                            \
	"\n"                                                                       \
	".PHONY: all setup build clean\n"                                          \
	"# ------------------------------------------------------------------ #\n"

#define lasm_template_entry_fmt                                                \
	"\n"                                                                       \
	"#define __alignment__ 8\n"                                                \
	"\n"                                                                       \
	"[addr=0, align=__alignment__, size=0, perm=rx,]\n"                        \
	"_meta:\n"                                                                 \
	"end\n"                                                                    \
	"\n"                                                                       \
	"[addr=auto, align=auto, size=auto, perm=auto,]\n"                         \
	"main:\n"                                                                  \
	"	nop\n"                                                                 \
	"	nop\n"                                                                 \
	"	nop\n"                                                                 \
	"end\n"
