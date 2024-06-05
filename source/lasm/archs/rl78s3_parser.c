
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

#include "lasm/archs/rl78s3_parser.h"
#include "lasm/debug.h"
#include "lasm/logger.h"

void rl78s3_parser_parse_tokens(lasm_lexer_s* const lexer, lasm_bytes_vector_s* const body)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(body != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, lexer->location);

	// note: https://llvm-gcc-renesas.com/pdf/r01us0015ej0220_rl78.pdf.

	while (!lasm_lexer_should_stop(lasm_lexer_lex(lexer, &token)))
	{
		if (lasm_token_type_keyword_end == token.type) return;

		// todo: implement!
		if ((lasm_token_type_ident == token.type) && (lasm_common_strcmp(token.as.ident.data, "nop") == 0))
		{
			// todo: implement!
			lasm_bytes_vector_push(body, 0x00);
			// todo: implement!
		}
		// todo: implement!

		lasm_logger_debug("%s", lasm_token_to_string(&token));
	}
}
