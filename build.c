
/**
 * @file build.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#define __build_c__
#include "build.h"

#define project_name  "lasm"
#define version_major 1
#define version_minor 0
#define version_patch 0
#define version_dev   0

#define _stringify(x) #x
#define stringify(x) _stringify(x)

#define static_array_length(_array) ((const uint64_t)(sizeof(_array) / sizeof(*_array)))

static const char_t* const sources[] =
{
	"./source/lasm/common.c",
	"./source/lasm/debug.c",
	"./source/lasm/logger.c",
	"./source/lasm/arena.c",
	"./source/lasm/config.c",
	"./source/lasm/utf8.c",
	"./source/lasm/token.c",
	"./source/lasm/lexer.c",
	"./source/lasm/ast.c",
	"./source/lasm/parser.c",
	"./source/lasm/archs/z80_parser.c",
	"./source/lasm/archs/rl78_parser.c",
	"./source/main.c",
};

typedef enum
{
	build_conf_debug,
	build_conf_release,
} build_conf_e;

static void make_compiler_command(build_command_s* const command, const build_conf_e conf);

static void make_linker_command(build_command_s* const command);

static void make_linter_command(build_command_s* const command, const build_conf_e conf);

static bool_t build(const build_conf_e conf);

static bool_t lint(const build_conf_e conf);

build_target(clean, "remove the build directory with all its artefacts.")
{
	build_command_s command = {0};
	build_command_append(&command, "rm", "-fr", "./build");
	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}

build_target(build_debug, "build the project in debug build configuration.")
{
	return build(build_conf_debug);
}

build_target(build_release, "build the project in release build configuration.")
{
	return build(build_conf_release);
}

build_target(docs, "generate the docs for the project.")
{
	build_command_s command = {0};
	build_command_append(&command, "doxygen", "./.doxyfile");
	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}

build_target(lint_debug, "lint debug build of the project.")
{
	return lint(build_conf_debug);
}

build_target(lint_release, "lint release build of the project.")
{
	return lint(build_conf_release);
}

build_targets(
	bind_target(clean        ),
	bind_target(build_debug  ),
	bind_target(build_release),
	bind_target(docs         ),
	bind_target(lint_debug   ),
	bind_target(lint_release ),
);

static void make_compiler_command(build_command_s* const command, const build_conf_e conf)
{
	build_command_append(command,
		"gcc",
		"-Wall",
		"-Wextra",
		"-Wpedantic",
		"-Werror",
		"-Wshadow",
		"-Wimplicit",
		"-Wreturn-type",
		"-Wunknown-pragmas",
		"-Wunused-variable",
		"-Wunused-function",
		"-Wmissing-prototypes",
		"-Wstrict-prototypes",
		"-Wconversion",
		"-Wsign-conversion",
		"-Wunreachable-code",
		"-std=gnu11",
		"-I./include",
		"-Dlasm_version_major=" stringify(version_major),
		"-Dlasm_version_minor=" stringify(version_minor),
		"-Dlasm_version_patch=" stringify(version_patch),
		"-Dlasm_version_dev=" stringify(version_dev),
	);

	switch (conf)
	{
		case build_conf_debug:
		{
			build_command_append(command,
				"-g3",
				"-O0",
			);
		} break;

		case build_conf_release:
		{
			build_command_append(command,
				"-O3",
				"-DNDEBUG",
			);
		} break;
	}
}

static void make_linker_command(build_command_s* const command)
{
	build_command_append(command,
		"gcc",
		"-o", "./build/"project_name,
		"-Wl,-Map=./build/"project_name".map",
		"-Wl,--cref",
	);
}

static void make_linter_command(build_command_s* const command, const build_conf_e conf)
{
	build_command_append(command,
		"cppcheck",
		"--enable=warning",
		"--inconclusive",
		"--std=c11",
		"--verbose",
		"--error-exitcode=1",
		"--language=c",
		"-I./include",
		"-Dlasm_version_major="stringify(version_major),
		"-Dlasm_version_minor="stringify(version_minor),
		"-Dlasm_version_patch="stringify(version_patch),
		"-Dlasm_version_dev="stringify(version_dev),
		"-j", "20",
	);

	switch (conf)
	{
		case build_conf_debug:
		{
		} break;

		case build_conf_release:
		{
			build_command_append(command,
				"-DNDEBUG",
			);
		} break;
	}
}

static bool_t build(const build_conf_e conf)
{
	build_command_s command = {0};
	bool_t status = true;

	// ----------------------------------
	// Prepare ./build directory.
	command.count = 0;
	build_command_append(&command, "sh", "-c", "find ./source -type d | cpio -pdmv ./build");
	if (!build_proc_run_sync(&command)) { status = false; goto build_end; }

	// ----------------------------------
	// Cleanup .elf artifacts.
	command.count = 0;
	build_command_append(&command, "rm", "-fr", "./build/"project_name".*");
	if (!build_proc_run_sync(&command)) { status = false; goto build_end; }

	// ----------------------------------
	// Compile source files.
	for (uint64_t index = 0; index < static_array_length(sources); ++index)
	{
		build_string_s object = {0};
		build_string_append(&object, "./build/", sources[index], ".o");

		command.count = 0;
		make_compiler_command(&command, conf);
		build_command_append(&command, "-c", sources[index], "-o", object.data);
		if (!build_proc_run_sync(&command)) { status = false; goto build_end; }
	}

	// ----------------------------------
	// Link object files.
	command.count = 0;
	make_linker_command(&command);
	for (uint64_t index = 0; index < static_array_length(sources); ++index)
	{
		build_string_s object = {0};
		build_string_append(&object, "./build/", sources[index], ".o");
		build_vector_append(&command, object.data);
	}
	if (!build_proc_run_sync(&command)) { status = false; goto build_end; }

	// ----------------------------------
	// Display file info of the .elf file.
	command.count = 0;
	build_command_append(&command, "file", "./build/"project_name);
	if (!build_proc_run_sync(&command)) { status = false; goto build_end; }

	// ----------------------------------
	// Display sizes of built binaries.
	command.count = 0;
	build_command_append(&command, "size", "--totals", "--format", "berkeley", "./build/"project_name);
	if (!build_proc_run_sync(&command)) { status = false; goto build_end; }

build_end:
	build_vector_drop(&command);
	return status;
}

static bool_t lint(const build_conf_e conf)
{
	build_command_s command = {0};
	make_linter_command(&command, conf);

	for (uint64_t index = 0; index < static_array_length(sources); ++index)
	{
		const char_t* const source = sources[index];
		const uint64_t source_length = strlen(source);
		if (source[source_length - 1] != 'c') continue;
		build_command_append(&command, sources[index]);
	}

	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}
