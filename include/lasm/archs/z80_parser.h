
/**
 * @file z80_parser.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-05
 */

#ifndef __lasm__include__lasm__archs__z80_parser_h__
#define __lasm__include__lasm__archs__z80_parser_h__

#include "lasm/common.h"
#include "lasm/lexer.h"
#include "lasm/ast.h"

void z80_parser_parse_tokens(lasm_lexer_s* const lexer, lasm_labels_vector_s* const labels, lasm_ast_label_s* const label);

#endif
