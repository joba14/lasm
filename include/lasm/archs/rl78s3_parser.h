
/**
 * @file x64_86_parser.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-05
 */

#ifndef __lasm__include__lasm__archs__x64_86_parser_h__
#define __lasm__include__lasm__archs__x64_86_parser_h__

#include "lasm/common.h"
#include "lasm/lexer.h"

void rl78s3_parser_parse_tokens(lasm_lexer_s* const lexer, lasm_bytes_vector_s* const body);

#endif
