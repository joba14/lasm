
/**
 * @file main.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#include "main.h"

#include "lasm/debug.h"
#include "lasm/logger.h"
#include "lasm/arena.h"
#include "lasm/config.h"
#include "lasm/parser.h"

#include <stdlib.h>

static void init(lasm_arena_s* const arena, lasm_config_init_s* const config);

static void build(lasm_arena_s* const arena, lasm_config_build_s* const config);

int32_t main(int32_t argc, const char_t** argv)
{
	lasm_debug_assert(argc > 0);
	lasm_debug_assert(argv != NULL);

	lasm_arena_s arena = lasm_arena_new();
	lasm_config_s config = lasm_config_from_cli(&arena, &argc, &argv);

	switch (config.type)
	{
		case lasm_config_type_init:
		{
			init(&arena, &config.as.init);
		} break;

		case lasm_config_type_build:
		{
			build(&arena, &config.as.build);
		} break;

		default:
		{
			lasm_debug_assert(0);  // note: should never happen.
		} break;
	}

	lasm_arena_drop(&arena);
	return 0;
}

static void init(lasm_arena_s* const arena, lasm_config_init_s* const config)
{
	lasm_debug_assert(arena != NULL);
	lasm_debug_assert(config != NULL);

	switch (config->template)
	{
		case lasm_template_type_make:
		{
			#include "./lasm/templates.inl"
			#define buffer_capacity 1024
			static char_t buffer[buffer_capacity + 1];

			(void)snprintf(buffer, buffer_capacity, "%s/makefile", config->directory);
			FILE* file = fopen(buffer, "w");

			if (NULL == file)
			{
				lasm_logger_error("failed to create build script file: %s.", buffer);
				lasm_common_exit(1);
				return;
			}

			(void)fprintf(file, "%s", lasm_template_makefile_fmt);
			(void)fclose(file);

			(void)snprintf(buffer, buffer_capacity, "%s/entry.lasm", config->directory);
			file = fopen(buffer, "w");

			if (NULL == file)
			{
				lasm_logger_error("failed to create entry.lasm file.");
				lasm_common_exit(1);
				return;
			}

			(void)fprintf(file, "%s", lasm_template_entry_fmt);
			(void)fclose(file);
		} break;

		default:
		{
			lasm_debug_assert(0);  // note: should never happen.
		} break;
	}
}

static void build(lasm_arena_s* const arena, lasm_config_build_s* const config)
{
	lasm_debug_assert(arena != NULL);
	lasm_debug_assert(config != NULL);

	lasm_parser_s parser = lasm_parser_new(arena, config);
	lasm_parser_shallow_parse(&parser);
	const lasm_labels_vector_s labels = lasm_parser_deep_parse(&parser);

	for (uint64_t index = 0; index < labels.count; ++index)
	{
		lasm_ast_label_s* const label = lasm_labels_vector_at((lasm_labels_vector_s* const)&labels, index);
		lasm_logger_info("\n%s\n", lasm_ast_label_to_string(label));
	}

	// todo: when building the final executable file of provided format, if the
	// provided format is not specific (e.g. elf or pe), it must be inferred by
	// the provided architecture.
	// todo: should check if any label matches the provided entry symbols.

	lasm_parser_drop(&parser);
}
