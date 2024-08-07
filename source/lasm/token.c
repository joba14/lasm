
/**
 * @file token.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#include "lasm/debug.h"
#include "lasm/logger.h"
#include "lasm/token.h"

#include <stdio.h>

static const char_t* const _g_token_type_to_string_map[] =
{
	[lasm_token_type_keyword_addr]				= "addr",
	[lasm_token_type_keyword_align]				= "align",
	[lasm_token_type_keyword_size]				= "size",
	[lasm_token_type_keyword_perm]				= "perm",
	[lasm_token_type_keyword_r]					= "r",
	[lasm_token_type_keyword_rw]				= "rw",
	[lasm_token_type_keyword_rx]				= "rx",
	[lasm_token_type_keyword_rwx]				= "rwx",
	[lasm_token_type_keyword_auto]				= "auto",
	[lasm_token_type_keyword_end]				= "end",

	[lasm_token_type_symbolic_dot]				= ".",
	[lasm_token_type_symbolic_comma]			= ",",
	[lasm_token_type_symbolic_equal]			= "=",
	[lasm_token_type_symbolic_colon]			= ":",
	[lasm_token_type_symbolic_left_bracket]		= "[",
	[lasm_token_type_symbolic_right_bracket]	= "]",
	[lasm_token_type_symbolic_plus]				= "+",
	[lasm_token_type_symbolic_minus]			= "-",
};

_Static_assert(
	lasm_token_type_keywords_count == 10,
	"_g_token_type_to_string_map is not in sync with lasm_token_type_e enum!"
);

_Static_assert(
	lasm_token_type_informationless_count == (sizeof(_g_token_type_to_string_map) / sizeof(_g_token_type_to_string_map[0])),
	"_g_token_type_to_string_map is not in sync with lasm_token_type_e enum!"
);

const char_t* lasm_token_type_to_string(const lasm_token_type_e type)
{
	switch (type)
	{
		case lasm_token_type_literal_uval: { return "uval";  } break;
		case lasm_token_type_literal_rune: { return "rune";  } break;
		case lasm_token_type_literal_str:  { return "str";   } break;
		case lasm_token_type_ident:        { return "ident"; } break;
		case lasm_token_type_eof:          { return "eof";   } break;
		case lasm_token_type_none:         { return "none";  } break;

		default:
		{
			lasm_debug_assert(type < (sizeof(_g_token_type_to_string_map) / sizeof(_g_token_type_to_string_map[0])));
			const char_t* const stringified_type = (const char_t* const)_g_token_type_to_string_map[type];
			lasm_debug_assert(stringified_type != NULL);
			return stringified_type;
		} break;
	}
}

lasm_token_s lasm_token_new(const lasm_token_type_e type, const lasm_location_s location)
{
	return (const lasm_token_s)
	{
		.type     = type,
		.location = location,
	};
}

const char_t* lasm_token_to_string(const lasm_token_s* const token)
{
	lasm_debug_assert(token != NULL);
	#define token_string_buffer_capacity 4096
	static char_t token_string_buffer[token_string_buffer_capacity + 1];

	uint64_t written = (uint64_t)snprintf(
		token_string_buffer, token_string_buffer_capacity,
		"Token[type='%s', location='" lasm_location_fmt "'",
		lasm_token_type_to_string(token->type),
		lasm_location_arg(token->location)
	);

	switch (token->type)
	{
		case lasm_token_type_literal_uval:
		{
			written += (uint64_t)snprintf(
				token_string_buffer + written, token_string_buffer_capacity - written,
				", data='%lu'", token->as.uval
			);
		} break;

		case lasm_token_type_literal_rune:
		{
			char_t encoded[lasm_utf8_max_size];
			const uint8_t size = lasm_utf8_encode(encoded, token->as.rune);

			written += (uint64_t)snprintf(
				token_string_buffer + written, token_string_buffer_capacity - written,
				", data='%.*s'", (int32_t)size, encoded
			);
		} break;

		case lasm_token_type_literal_str:
		{
			lasm_debug_assert(token->as.str.data != NULL);
			lasm_debug_assert(token->as.str.length > 0);

			written += (uint64_t)snprintf(
				token_string_buffer + written, token_string_buffer_capacity - written,
				", data='%.*s'", (int32_t)token->as.str.length, token->as.str.data
			);
		} break;

		case lasm_token_type_ident:
		{
			lasm_debug_assert(token->as.ident.data != NULL);
			lasm_debug_assert(token->as.ident.length > 0);

			written += (uint64_t)snprintf(
				token_string_buffer + written, token_string_buffer_capacity - written,
				", data='%.*s'", (int32_t)token->as.ident.length, token->as.ident.data
			);
		} break;

		default:
		{
		} break;
	}

	written += (uint64_t)snprintf(
		token_string_buffer + written, token_string_buffer_capacity - written, "]"
	);

	token_string_buffer[written] = 0;
	return token_string_buffer;
}

lasm_implement_vector_type(lasm_tokens_vector, lasm_token_s);
