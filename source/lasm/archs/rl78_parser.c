
/**
 * @file rl78_parser.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-07-21
 */

#include "lasm/archs/rl78_parser.h"
#include "lasm/debug.h"
#include "lasm/logger.h"

void rl78_parser_parse_tokens(lasm_lexer_s* const lexer, lasm_labels_vector_s* const labels, lasm_ast_label_s* const label)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(labels != NULL);
	lasm_debug_assert(label != NULL);

	// note: https://llvm-gcc-renesas.com/pdf/r01us0015ej0220_rl78.pdf.

	for (uint64_t index = 0; index < label->body_tokens.count; ++index)
	{
		lasm_token_s* const token = lasm_tokens_vector_at(&label->body_tokens, index);
		(void)token;
	}
}
