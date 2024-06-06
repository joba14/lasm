
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

#define PROJECT_NAME "lasm"

static void cc_flags(build_command_s* const command)
{
	build_vector_append(command, "cc");
	build_vector_append(command, "-Wall");
	build_vector_append(command, "-Wextra");
	build_vector_append(command, "-Wpedantic");
	build_vector_append(command, "-Werror");
	build_vector_append(command, "-Wshadow");
	build_vector_append(command, "-Wimplicit");
	build_vector_append(command, "-Wreturn-type");
	build_vector_append(command, "-Wunknown-pragmas");
	build_vector_append(command, "-Wunused-variable");
	build_vector_append(command, "-Wunused-function");
	build_vector_append(command, "-Wmissing-prototypes");
	build_vector_append(command, "-Wstrict-prototypes");
	build_vector_append(command, "-Wconversion");
	build_vector_append(command, "-Wsign-conversion");
	build_vector_append(command, "-Wunreachable-code");
	build_vector_append(command, "-std=gnu11");
	build_vector_append(command, "-I./include");
}

static void debug_flags(build_command_s* const command)
{
	build_vector_append(command, "-g");
	build_vector_append(command, "-O0");
	build_vector_append(command, "-o");
	build_vector_append(command, "./build/debug/"PROJECT_NAME);
}

static void release_flags(build_command_s* const command)
{
	build_vector_append(command, "-O3");
	build_vector_append(command, "-DNDEBUG");
	build_vector_append(command, "-o");
	build_vector_append(command, "./build/release/"PROJECT_NAME);
}

static void sources(build_command_s* const command)
{
	build_vector_append(command, "./source/lasm/common.c");
	build_vector_append(command, "./source/lasm/debug.c");
	build_vector_append(command, "./source/lasm/logger.c");
	build_vector_append(command, "./source/lasm/arena.c");
	build_vector_append(command, "./source/lasm/config.c");
	build_vector_append(command, "./source/lasm/utf8.c");
	build_vector_append(command, "./source/lasm/token.c");
	build_vector_append(command, "./source/lasm/lexer.c");
	build_vector_append(command, "./source/lasm/ast.c");
	build_vector_append(command, "./source/lasm/parser.c");
	build_vector_append(command, "./source/lasm/archs/rl78s3_parser.c");
	build_vector_append(command, "./source/main.c");
}

build_target(clean, "remove the build directory with all its artefacts.")
{
	build_command_s command = {0};
	build_command_append(&command, "rm", "-fr", "./build");
	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}

build_target(prep, "create build directory.")
{
	build_command_s command = {0};
	build_command_append(&command, "mkdir", "./build", "./build/debug", "./build/release");
	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}

build_target(debug, "build the project in debug build configuration.")
{
	build_command_s command = {0};
	cc_flags(&command);
	debug_flags(&command);
	sources(&command);
	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}

build_target(release, "build the project in release build configuration.")
{
	build_command_s command = {0};
	cc_flags(&command);
	release_flags(&command);
	sources(&command);
	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}

build_target(run_debug, "run the project in debug configuration.")
{
	build_command_s command = {0};
	build_command_append(&command, "./build/debug/"PROJECT_NAME, "build", "-a", "rl78s3", "-f", "elf", "./examples/experiments.lasm");
	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}

build_target(run_release, "run the project in release configuration.")
{
	build_command_s command = {0};
	build_command_append(&command, "./build/release/"PROJECT_NAME, "build", "-a", "rl78s3", "-f", "elf", "./examples/experiments.lasm");
	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}

build_target(docs, "generate the docs for the project.")
{
	build_command_s command = {0};
	build_command_append(&command, "doxygen", "./doxyfile.sh");
	const bool_t status = build_proc_run_sync(&command);
	build_vector_drop(&command);
	return status;
}

build_target(all, "run all targets one after the other.")
{
	return clean() && prep() && debug() && release();
}

build_targets(
	bind_target(clean      ),
	bind_target(prep       ),
	bind_target(debug      ),
	bind_target(release    ),
	bind_target(run_debug  ),
	bind_target(run_release),
	bind_target(docs       ),
	bind_target(all        ),
);
