
/**
 * @file token.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#ifndef __lasm__include__lasm__token_h__
#define __lasm__include__lasm__token_h__

#include "lasm/common.h"
#include "lasm/arena.h"
#include "lasm/vector.h"
#include "lasm/utf8.h"

typedef struct
{
	const char_t* file;
	uint64_t      line;
	uint64_t      column;
} lasm_location_s;

#define lasm_location_fmt "%s:%lu:%lu"
#define lasm_location_arg(_location) (_location).file, (_location).line, (_location).column

typedef enum
{
	// Reserved keyword tokens
	lasm_token_type_keyword_entry,				// entry
	lasm_token_type_keyword_addr,				// addr
	lasm_token_type_keyword_align,				// align
	lasm_token_type_keyword_size,				// size
	lasm_token_type_keyword_perm,				// perm
	lasm_token_type_keyword_r,					// r
	lasm_token_type_keyword_rw,					// rw
	lasm_token_type_keyword_rx,					// rx
	lasm_token_type_keyword_rwx,				// rwx
	lasm_token_type_keyword_auto,				// auto
	lasm_token_type_keyword_end,				// end
	lasm_token_type_keywords_count,

	// Symbolic tokens
	lasm_token_type_symbolic_dot				// .
		= lasm_token_type_keywords_count,
	lasm_token_type_symbolic_comma,				// ,
	lasm_token_type_symbolic_equal,				// =
	lasm_token_type_symbolic_colon,				// :
	lasm_token_type_symbolic_left_bracket,		// [
	lasm_token_type_symbolic_right_bracket,		// ]
	lasm_token_type_informationless_count,

	// Tokens with additional information
	lasm_token_type_literal_uval
		= lasm_token_type_informationless_count,
	lasm_token_type_literal_rune,
	lasm_token_type_literal_str,
	lasm_token_type_ident,

	// Magic tokens
	lasm_token_type_eof,
	lasm_token_type_none
} lasm_token_type_e;

const char_t* lasm_token_type_to_string(const lasm_token_type_e type);

typedef struct
{
	lasm_token_type_e type;
	lasm_location_s location;

	union
	{
		uint64_t uval;
		utf8char_t rune;

		struct
		{
			char_t*  data;
			uint64_t length;
		} str;

		struct
		{
			char_t*  data;
			uint64_t length;
		} ident;
	} as;
} lasm_token_s;

/**
 * @brief Create new token object with provided type and location.
 * 
 * @param type     type to assign to the token
 * @param location location to assign to the token
 * 
 * @return lasm_token_s
 */
lasm_token_s lasm_token_new(const lasm_token_type_e type, const lasm_location_s location);

/**
 * @brief Stringify token.
 * 
 * @warning This function uses a static internal buffer for stringified tokens.
 * Use returned reference before calling this function again, as with each call
 * the internal buffer gets modified.
 * 
 * @param token token to stringify
 * 
 * @return const char_t*
 */
const char_t* lasm_token_to_string(const lasm_token_s* const token);

lasm_define_vector_type(lasm_bytes_vector, uint8_t);

#endif
