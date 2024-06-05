
/**
 * @file x64_86_parser.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-05
 */

#include "lasm/archs/x64_86_parser.h"
#include "lasm/debug.h"
#include "lasm/logger.h"

void x64_86_parser_parse_tokens(lasm_lexer_s* const lexer, lasm_bytes_vector_s* const body)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(body != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, lexer->location);

	while (!lasm_lexer_should_stop(lasm_lexer_lex(lexer, &token)))
	{
		if (lasm_token_type_keyword_end == token.type) return;

		if ((lasm_token_type_ident == token.type) && (lasm_common_strcmp(token.as.ident.data, "nop") == 0))
		{
			lasm_bytes_vector_push(body, 0x00);
		}

		lasm_logger_debug("%s", lasm_token_to_string(&token));
	}
}
