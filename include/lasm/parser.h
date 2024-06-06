
/**
 * @file parser.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#ifndef __lasm__include__lasm__parser_h__
#define __lasm__include__lasm__parser_h__

#include "lasm/common.h"
#include "lasm/arena.h"
#include "lasm/config.h"
#include "lasm/lexer.h"
#include "lasm/ast.h"

typedef struct
{
	lasm_arena_s* arena;
	lasm_config_s* config;
	lasm_lexer_s lexer;
	lasm_labels_vector_s labels;
} lasm_parser_s;

/**
 * @brief Create a parser.
 * 
 * @param arena     arena reference
 * @param file_path path to file to be bound to the parser
 * 
 * @return lasm_parser_s
 */
lasm_parser_s lasm_parser_new(lasm_arena_s* const arena, lasm_config_s* const config);

/**
 * @brief Drop the parser and close the file that is bound to it.
 * 
 * @param parser parser reference
 */
void lasm_parser_drop(lasm_parser_s* const parser);

void lasm_parser_shallow_parse(lasm_parser_s* const parser);

lasm_labels_vector_s lasm_parser_deep_parse(lasm_parser_s* const parser);

#endif
