
/**
 * @file parser.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#include "lasm/parser.h"
#include "lasm/debug.h"
#include "lasm/logger.h"

#define log_parser_error(_location, _format, ...)                              \
	do                                                                         \
	{                                                                          \
		(void)fprintf(stderr, "%s:%lu:%lu: ",                                  \
			(_location).file, (_location).line, (_location).column);           \
		lasm_logger_error(_format, ## __VA_ARGS__);                            \
		lasm_common_exit(1);                                                   \
	} while (0)

static void parse_label_attr(lasm_parser_s* const parser, lasm_ast_label_s* const label, const lasm_ast_attr_type_e attr_type);

static void parse_label_attr_addr(lasm_parser_s* const parser, lasm_ast_label_s* const label);
static void parse_label_attr_align(lasm_parser_s* const parser, lasm_ast_label_s* const label);
static void parse_label_attr_size(lasm_parser_s* const parser, lasm_ast_label_s* const label);
static void parse_label_attr_perm(lasm_parser_s* const parser, lasm_ast_label_s* const label);

lasm_parser_s lasm_parser_new(lasm_arena_s* const arena, const char_t* const file_path)
{
	lasm_debug_assert(arena != NULL);
	lasm_debug_assert(file_path != NULL);

	const lasm_lexer_s lexer = lasm_lexer_new(arena, file_path);

	return (lasm_parser_s)
	{
		.arena = arena,
		.lexer = lexer,
	};
}

void lasm_parser_drop(lasm_parser_s* const parser)
{
	lasm_debug_assert(parser != NULL);
	lasm_lexer_drop(&parser->lexer);
}

lasm_ast_label_s* lasm_parser_parse_label(lasm_parser_s* const parser)
{
	lasm_debug_assert(parser != NULL);

	lasm_ast_label_s* const label = lasm_arena_alloc(parser->arena, sizeof(*label));
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_left_bracket)
	{
		log_parser_error(token.location,
			"expected a symbolic token '[', but found '%s' token. all global definitions must be labels which start\n"
			"with the attributes list. follow the example below:\n"
			"    " lasm_red "v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    " lasm_red "[" lasm_reset "addr=<value>, align=<value>, size=<value>, perm=<value>]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	parse_label_attr(parser, label, lasm_ast_attr_type_addr);
	parse_label_attr(parser, label, lasm_ast_attr_type_align);
	parse_label_attr(parser, label, lasm_ast_attr_type_size);
	parse_label_attr(parser, label, lasm_ast_attr_type_perm);
	return label;
}

static void parse_label_attr(lasm_parser_s* const parser, lasm_ast_label_s* const label, const lasm_ast_attr_type_e attr_type)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	switch (attr_type)
	{
		case lasm_ast_attr_type_addr:  { parse_label_attr_addr(parser, label);  } break;
		case lasm_ast_attr_type_align: { parse_label_attr_align(parser, label); } break;
		case lasm_ast_attr_type_size:  { parse_label_attr_size(parser, label);  } break;
		case lasm_ast_attr_type_perm:  { parse_label_attr_perm(parser, label);  } break;

		default:
		{
			// note: should never ever happen as the valid attributes are all handled in the cases.
			lasm_debug_assert(0);  // note: sanity check for developers.
		} break;
	}
}

static void parse_label_attr_addr(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_keyword_addr)
	{
		log_parser_error(token.location,
			"expected a 'addr' keyword after symbolic token '[', but found '%s' token. the attributes list have a\n"
			"specific order that must be followed. follow the example below:\n"
			"    " lasm_red " v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[" lasm_red "addr" lasm_reset "=<value>, align=<value>, size=<value>, perm=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_equal)
	{
		log_parser_error(token.location,
			"expected a '=' symbol token after 'addr' keyword, but found '%s' token. each attribute in the list of\n"
			"attributes must have a value assigned to it. follow the example below:\n"
			"    " lasm_red "     v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr" lasm_red "=" lasm_reset "<value>, align=<value>, size=<value>, perm=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	(void)lasm_lexer_lex(&parser->lexer, &token);
	if ((token.type != lasm_token_type_keyword_auto) && (token.type != lasm_token_type_literal_uval))
	{
		log_parser_error(token.location,
			"expected an 'auto' keyword or a numeric value for the 'addr' attribute, but found '%s' token.",
			lasm_token_type_to_string(token.type)
		);
	}

	label->attrs[lasm_ast_attr_type_addr] = (const lasm_ast_attr_s)
	{
		.type     = lasm_ast_attr_type_addr,
		.inferred = (lasm_token_type_keyword_auto == token.type),
		.as.addr  = (const lasm_ast_attr_addr_s)
		{
			.value = (lasm_token_type_keyword_auto == token.type ? 0 : token.uval),
		},
	};

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_comma)
	{
		log_parser_error(token.location,
			"expected a ',' symbolic token after the 'addr' attribute's value, but found '%s' token.\n"
			"attributes must have a trailing ',' after their values. follow the example below:\n"
			"    " lasm_red "             v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>" lasm_red "," lasm_reset " align=<value>, size=<value>, perm=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}
}

static void parse_label_attr_align(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_keyword_align)
	{
		log_parser_error(token.location,
			"expected a 'align' keyword as the second attribute, but found '%s' token. the attributes list have a\n"
			"specific order that must be followed. follow the example below:\n"
			"    " lasm_red "               v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>, " lasm_red "align" lasm_reset "=<value>, size=<value>, perm=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_equal)
	{
		log_parser_error(token.location,
			"expected a '=' symbol token after 'align' keyword, but found '%s' token. each attribute in the list of\n"
			"attributes must have a value assigned to it. follow the example below:\n"
			"    " lasm_red "          v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>, align" lasm_red "=" lasm_reset "<value>, size=<value>, perm=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	(void)lasm_lexer_lex(&parser->lexer, &token);
	if ((token.type != lasm_token_type_keyword_auto) && (token.type != lasm_token_type_literal_uval))
	{
		log_parser_error(token.location,
			"expected an 'auto' keyword or a numeric value for the 'align' attribute, but found '%s' token.",
			lasm_token_type_to_string(token.type)
		);
	}

	if (lasm_token_type_literal_uval == token.type)
	{
		if (token.uval > 8)
		{
			log_parser_error(token.location,
				"align attribute value cannot exceed 8, but found value %lu specified for align attribute.",
				token.uval
			);
		}
	}

	label->attrs[lasm_ast_attr_type_align] = (const lasm_ast_attr_s)
	{
		.type     = lasm_ast_attr_type_align,
		.inferred = (lasm_token_type_keyword_auto == token.type),
		.as.align = (const lasm_ast_attr_align_s)
		{
			.value = (lasm_token_type_keyword_auto == token.type ? 0 : token.uval),
		},
	};

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_comma)
	{
		log_parser_error(token.location,
			"expected a ',' symbolic token after the 'align' attribute's value, but found '%s' token.\n"
			"attributes must have a trailing ',' after their values. follow the example below:\n"
			"    " lasm_red "                            v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>, align=<value>" lasm_red "," lasm_reset " size=<value>, perm=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}
}

static void parse_label_attr_size(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_keyword_size)
	{
		log_parser_error(token.location,
			"expected a 'size' keyword as the third attribute, but found '%s' token. the attributes list have a\n"
			"specific order that must be followed. follow the example below:\n"
			"    " lasm_red "                              v~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>, align=<value>, " lasm_red "size" lasm_reset "=<value>, perm=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_equal)
	{
		log_parser_error(token.location,
			"expected a '=' symbol token after 'size' keyword, but found '%s' token. each attribute in the list of\n"
			"attributes must have a value assigned to it. follow the example below:\n"
			"    " lasm_red "                                  v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>, align=<value>, size" lasm_red "=" lasm_reset "<value>, perm=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	(void)lasm_lexer_lex(&parser->lexer, &token);
	if ((token.type != lasm_token_type_keyword_auto) && (token.type != lasm_token_type_literal_uval))
	{
		log_parser_error(token.location,
			"expected an 'auto' keyword or a numeric value for the 'size' attribute, but found '%s' token.",
			lasm_token_type_to_string(token.type)
		);
	}

	label->attrs[lasm_ast_attr_type_size] = (const lasm_ast_attr_s)
	{
		.type     = lasm_ast_attr_type_size,
		.inferred = (lasm_token_type_keyword_auto == token.type),
		.as.size  = (const lasm_ast_attr_size_s)
		{
			.value = (lasm_token_type_keyword_auto == token.type ? 0 : token.uval),
		},
	};

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_comma)
	{
		log_parser_error(token.location,
			"expected a ',' symbolic token after the 'size' attribute's value, but found '%s' token.\n"
			"attributes must have a trailing ',' after their values. follow the example below:\n"
			"    " lasm_red "                                          v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>, align=<value>, size=<value>" lasm_red "," lasm_reset " perm=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}
}

static void parse_label_attr_perm(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_keyword_perm)
	{
		log_parser_error(token.location,
			"expected a 'perm' keyword as the third attribute, but found '%s' token. the attributes list have a\n"
			"specific order that must be followed. follow the example below:\n"
			"    " lasm_red "                                            v~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>, align=<value>, size=<value>, " lasm_red "perm" lasm_reset "=<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_equal)
	{
		log_parser_error(token.location,
			"expected a '=' symbol token after 'perm' keyword, but found '%s' token. each attribute in the list of\n"
			"attributes must have a value assigned to it. follow the example below:\n"
			"    " lasm_red "                                                v~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>, align=<value>, size=<value>, perm" lasm_red "=" lasm_reset "<value>,]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	if ((token.type != lasm_token_type_keyword_r  ) &&
		(token.type != lasm_token_type_keyword_rw ) &&
		(token.type != lasm_token_type_keyword_rx ) &&
		(token.type != lasm_token_type_keyword_rwx))
	{
		
	}

	lasm_ast_perm_type_e perm_type = lasm_ast_perm_type_none;
	(void)lasm_lexer_lex(&parser->lexer, &token);

	switch (token.type)
	{
		case lasm_token_type_keyword_r:   { perm_type = lasm_ast_perm_type_r;   } break;
		case lasm_token_type_keyword_rw:  { perm_type = lasm_ast_perm_type_rw;  } break;
		case lasm_token_type_keyword_rx:  { perm_type = lasm_ast_perm_type_rx;  } break;
		case lasm_token_type_keyword_rwx: { perm_type = lasm_ast_perm_type_rwx; } break;

		default:
		{
			log_parser_error(token.location,
				"expected an 'auto' keyword or any of the 'r', 'rw', 'rx', or 'rwx' keywords for the 'perm' attribute, but found '%s' token.",
				lasm_token_type_to_string(token.type)
			);
		} break;
	}

	label->attrs[lasm_ast_attr_type_perm] = (const lasm_ast_attr_s)
	{
		.type     = lasm_ast_attr_type_perm,
		.inferred = (lasm_token_type_keyword_auto == token.type),
		.as.perm  = (const lasm_ast_attr_perm_s)
		{
			.value = (lasm_token_type_keyword_auto == token.type ? lasm_ast_perm_type_none : perm_type),
		},
	};

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_comma)
	{
		log_parser_error(token.location,
			"expected a ',' symbolic token after the 'perm' attribute's value, but found '%s' token.\n"
			"attributes must have a trailing ',' after their values. follow the example below:\n"
			"    " lasm_red "                                                        v~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "[addr=<value>, align=<value>, size=<value>, perm=<value>" lasm_red "," lasm_reset "]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}
}