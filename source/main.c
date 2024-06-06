
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

int32_t main(int32_t argc, const char_t** argv)
{
	lasm_debug_assert(argc > 0);
	lasm_debug_assert(argv != NULL);

	lasm_arena_s arena = lasm_arena_new();
	lasm_config_s config = lasm_config_from_cli(&arena, &argc, &argv);
	lasm_parser_s parser = lasm_parser_new(&arena, &config);

	lasm_labels_vector_s labels = lasm_parser_shallow_parse(&parser);
	lasm_parser_deep_parse(&parser, &labels);

	lasm_parser_drop(&parser);
	lasm_arena_drop(&arena);
	return 0;
}
