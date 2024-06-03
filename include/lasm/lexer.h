
/**
 * @file lexer.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#ifndef __lasm__include__lasm__lexer_h__
#define __lasm__include__lasm__lexer_h__

#include "lasm/common.h"
#include "lasm/arena.h"
#include "lasm/utf8.h"
#include "lasm/token.h"

typedef struct
{
	lasm_arena_s* arena;
	FILE* file;
	lasm_location_s location;
	lasm_token_s token;
	utf8char_t cache[2];

	struct
	{
		uint64_t capacity;
		char_t*  data;
		uint64_t length;
	} buffer;
} lasm_lexer_s;

/**
 * @brief Create a lexer.
 * 
 * @param arena     arena reference
 * @param file_path path to file to be bound to the lexer
 * 
 * @return lasm_lexer_s
 */
lasm_lexer_s lasm_lexer_new(lasm_arena_s* const arena, const char_t* const file_path);

/**
 * @brief Drop the lexer and close the file that is bound to it.
 * 
 * @param lexer lexer reference
 */
void lasm_lexer_drop(lasm_lexer_s* const lexer);

/**
 * @brief Lex next token
 * 
 * @param lexer lexer reference
 * @param token token reference
 * 
 * @return lasm_token_type_e
 */
lasm_token_type_e lasm_lexer_lex(lasm_lexer_s* const lexer, lasm_token_s* const token);

/**
 * @brief Check if lexer should stop as there are no more tokens to lex in the
 * bound file.
 * 
 * @param type token type to check against
 * 
 * @return bool_t
 */
bool_t lasm_lexer_should_stop(const lasm_token_type_e type);

/**
 * @brief Unlex (cache back) a token.
 * 
 * @param lexer lexer reference
 * @param token token to unlex
 */
void lasm_lexer_unlex(lasm_lexer_s* const lexer, const lasm_token_s* const token);

#endif
