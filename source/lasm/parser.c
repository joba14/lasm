
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

static void parse_label_attr_addr(lasm_parser_s* const parser, lasm_ast_label_s* const label);

static void parse_label_attr_align(lasm_parser_s* const parser, lasm_ast_label_s* const label);

static void parse_label_attr_size(lasm_parser_s* const parser, lasm_ast_label_s* const label);

static void parse_label_attr_perm(lasm_parser_s* const parser, lasm_ast_label_s* const label);

static bool_t parse_label_body(lasm_parser_s* const parser, lasm_ast_label_s* const label);

const char_t* lasm_ast_perm_type_to_string(const lasm_ast_perm_type_e type)
{
	switch (type)
	{
		case lasm_ast_perm_type_r:   { return "r";   } break;
		case lasm_ast_perm_type_rw:  { return "rw";  } break;
		case lasm_ast_perm_type_rx:  { return "rx";  } break;
		case lasm_ast_perm_type_rwx: { return "rwx"; } break;

		default:
		{
			lasm_debug_assert(0);  // note: sanity check for developers.
			return NULL;
		} break;
	}
}

const char_t* lasm_ast_label_to_string(const lasm_ast_label_s* const label)
{
	lasm_debug_assert(label != NULL);

	#define label_string_buffer_capacity 4096
	static char_t label_string_buffer[label_string_buffer_capacity + 1];
	uint64_t written = 0;

	const lasm_ast_attr_s addr_attr = label->attrs[lasm_ast_attr_type_addr];
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "[addr=");
	if (addr_attr.inferred) written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "auto");
	else written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%lu", addr_attr.as.addr.value);
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", ", ");

	const lasm_ast_attr_s align_attr = label->attrs[lasm_ast_attr_type_align];
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "align=");
	if (align_attr.inferred) written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "auto");
	else written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%lu", align_attr.as.align.value);
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", ", ");

	const lasm_ast_attr_s size_attr = label->attrs[lasm_ast_attr_type_size];
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "size=");
	if (size_attr.inferred) written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "auto");
	else written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%lu", size_attr.as.size.value);
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", ", ");

	const lasm_ast_attr_s perm_attr = label->attrs[lasm_ast_attr_type_perm];
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "perm=");
	if (perm_attr.inferred) written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "auto");
	else written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", lasm_ast_perm_type_to_string(perm_attr.as.perm.value));
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", ",]\n");

	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s:\n", label->name);
	for (uint64_t index = 0; index < label->body.count; ++index) written += (uint64_t)snprintf(label_string_buffer + written,
	label_string_buffer_capacity - written, "    %s\n", lasm_token_to_string(lasm_tokens_vector_at((lasm_tokens_vector_s* const)&label->body, index)));
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "end");

	return label_string_buffer;
}

lasm_parser_s lasm_parser_new(lasm_arena_s* const arena, lasm_config_s* const config)
{
	lasm_debug_assert(arena != NULL);
	lasm_debug_assert(config != NULL);

	return (lasm_parser_s)
	{
		.arena  = arena,
		.config = config,
		.lexer  = lasm_lexer_new(arena, config),
	};
}

void lasm_parser_drop(lasm_parser_s* const parser)
{
	lasm_debug_assert(parser != NULL);
	lasm_lexer_drop(&parser->lexer);
}

bool_t lasm_parser_parse_label(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);
	(void)lasm_lexer_lex(&parser->lexer, &token);

	if (lasm_lexer_should_stop(token.type))
	{
		return false;
	}

	if (token.type != lasm_token_type_symbolic_left_bracket)
	{
		log_parser_error(token.location,
			"expected a symbolic token '[', but found '%s' token. all global definitions must be labels which start\n"
			"with the attributes list. follow the example below:\n"
			"    " lasm_red "v~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    " lasm_red "[" lasm_reset "addr=<value>, align=<value>, size=<value>, perm=<value>]\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	parse_label_attr_addr(parser, label);
	parse_label_attr_align(parser, label);
	parse_label_attr_size(parser, label);
	parse_label_attr_perm(parser, label);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_right_bracket)
	{
		log_parser_error(token.location,
			"expected a symbolic token ']' after the attributes list, but found '%s' token. an attributes list must be close\n"
			"with ']' symbolic token. follow the example below:\n"
			"    " lasm_red "                                             ~~~~~~~~~~~v" lasm_reset "\n"
			"    [addr=<value>, align=<value>, size=<value>, perm=<value>" lasm_red "]" lasm_reset "\n"
			"    "          "example:\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_ident)
	{
		log_parser_error(token.location,
			"expected an identifier token after attributes list for the label, but found '%s' token. a label name must follow\n"
			"the attributes list. follow the example below:\n"
			lasm_red " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			lasm_red "|  " lasm_reset " [addr=<value>, align=<value>, size=<value>, perm=<value>]\n"
			lasm_red " ~> example" lasm_reset ":\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	label->name = token.as.ident.data;

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_colon)
	{
		log_parser_error(token.location,
			"expected a ':' symbolic token after the label's identifier token, but found '%s' token. a ':' symbolic token must\n"
			"follow the label's identifier/name token. follow the example below:\n"
			"                                                                  " lasm_red "|" lasm_reset "\n"
			"    [addr=<value>, align=<value>, size=<value>, perm=<value>]     " lasm_red "|" lasm_reset "\n"
			"    "          "example" lasm_red ": <~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" lasm_reset "\n"
			"    "          "    ; ... \n"
			"    "          "end\n",
			lasm_token_type_to_string(token.type)
		);
	}

	return parse_label_body(parser, label);
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
			.value = (lasm_token_type_keyword_auto == token.type ? 0 : token.as.uval),
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
		if (token.as.uval > 8)
		{
			log_parser_error(token.location,
				"align attribute value cannot exceed 8, but found value %lu specified for align attribute.",
				token.as.uval
			);
		}
	}

	label->attrs[lasm_ast_attr_type_align] = (const lasm_ast_attr_s)
	{
		.type     = lasm_ast_attr_type_align,
		.inferred = (lasm_token_type_keyword_auto == token.type),
		.as.align = (const lasm_ast_attr_align_s)
		{
			.value = (lasm_token_type_keyword_auto == token.type ? 0 : token.as.uval),
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
			.value = (lasm_token_type_keyword_auto == token.type ? 0 : token.as.uval),
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

static bool_t parse_label_body(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	label->body = lasm_tokens_vector_new(parser->arena, 1);
	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	switch (lasm_arch_type_from_string(parser->config->arch))
	{
		case lasm_arch_type_x64_86:
		{
			// todo: parse the x64_86 assembly!
		} break;

		case lasm_arch_type_arn16:
		{
			// todo: parse the arn16 assembly!
		} break;

		default:
		{
			lasm_debug_assert(0);
		} break;
	}

	while (!lasm_lexer_should_stop(lasm_lexer_lex(&parser->lexer, &token)))
	{
		if (lasm_token_type_keyword_end == token.type) break;
		lasm_tokens_vector_push(&label->body, token);
	}

	return true;
}
