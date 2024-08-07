
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
#include "lasm/archs/z80_parser.h"
#include "lasm/archs/rl78_parser.h"

#define _log_parser_note(_location, _format, ...)                              \
	do                                                                         \
	{                                                                          \
		(void)fprintf(stderr, "%s:%lu:%lu: ",                                  \
			(_location).file, (_location).line, (_location).column);           \
		lasm_logger_note(_format, ## __VA_ARGS__);                             \
	} while (0)

#define _log_parser_warn(_location, _format, ...)                              \
	do                                                                         \
	{                                                                          \
		(void)fprintf(stderr, "%s:%lu:%lu: ",                                  \
			(_location).file, (_location).line, (_location).column);           \
		lasm_logger_warn(_format, ## __VA_ARGS__);                             \
	} while (0)

#define _log_parser_error(_location, _format, ...)                             \
	do                                                                         \
	{                                                                          \
		(void)fprintf(stderr, "%s:%lu:%lu: ",                                  \
			(_location).file, (_location).line, (_location).column);           \
		lasm_logger_error(_format, ## __VA_ARGS__);                            \
		lasm_common_exit(1);                                                   \
	} while (0)

static void _parse_label_attr_addr(lasm_parser_s* const parser, lasm_ast_label_s* const label);

static void _parse_label_attr_align(lasm_parser_s* const parser, lasm_ast_label_s* const label);

static void _parse_label_attr_size(lasm_parser_s* const parser, lasm_ast_label_s* const label);

static void _parse_label_attr_perm(lasm_parser_s* const parser, lasm_ast_label_s* const label);

static bool_t _parse_label_header(lasm_parser_s* const parser, lasm_ast_label_s* const label);

static void _parse_label_body(lasm_parser_s* const parser, lasm_ast_label_s* const label);

lasm_parser_s lasm_parser_new(lasm_arena_s* const arena, lasm_config_build_s* const config)
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

void lasm_parser_shallow_parse(lasm_parser_s* const parser)
{
	lasm_debug_assert(parser != NULL);

	parser->labels = lasm_labels_vector_new(parser->arena, 1);
	lasm_ast_label_s label = {0};

	while (_parse_label_header(parser, &label))
	{
		lasm_labels_vector_push(&parser->labels, label);
	}
}

lasm_labels_vector_s lasm_parser_deep_parse(lasm_parser_s* const parser)
{
	lasm_debug_assert(parser != NULL);

	for (uint64_t index = 0; index < parser->labels.count; ++index)
	{
		lasm_ast_label_s* const label = lasm_labels_vector_at(&parser->labels, index);
		_parse_label_body(parser, label);
	}

	return parser->labels;
}

static void _parse_label_attr_addr(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_keyword_addr)
	{
		_log_parser_error(token.location,
			"expected a 'addr' keyword after symbolic token '[', but found '%s' token. the attributes list have a specific order that must be followed. follow the example below:\n" \
			"  |                                                                                                                                                                 \n" \
			"2 |     [%saddr%s=<value>, align=<value>, size=<value>, perm=<value>,]                                                                                              \n" \
			"  |      %s^~~~ expected the 'addr' keyword as the first attribute in the attributes list.%s                                                                        \n" \
			"3 |     example:                                                                                                                                                    \n" \
			"4 |         ; ...                                                                                                                                                   \n" \
			"5 |     end                                                                                                                                                         \n" \
			"  |                                                                                                                                                                 \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_equal)
	{
		_log_parser_error(token.location,
			"expected a '=' symbol token after 'addr' keyword, but found '%s' token. each attribute in the list of attributes must have a value assigned to it. follow the example below:\n" \
			"  |                                                                                                                                                                         \n" \
			"2 |     [addr%s=%s<value>, align=<value>, size=<value>, perm=<value>,]                                                                                                      \n" \
			"  |          %s^~~~ expected a '=' symbol to follow after the 'addr' keyword.%s                                                                                             \n" \
			"3 |     example:                                                                                                                                                            \n" \
			"4 |         ; ...                                                                                                                                                           \n" \
			"5 |     end                                                                                                                                                                 \n" \
			"  |                                                                                                                                                                         \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	(void)lasm_lexer_lex(&parser->lexer, &token);

	if ((token.type != lasm_token_type_keyword_auto) && (token.type != lasm_token_type_literal_uval))
	{
		_log_parser_error(token.location,
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
		_log_parser_error(token.location,
			"expected a ',' symbolic token after the 'addr' attribute's value, but found '%s' token. attributes must have a trailing ',' after their values. follow the example below:\n" \
			"  |                                                                                                                                                                      \n" \
			"2 |     [addr=<value>%s,%s align=<value>, size=<value>, perm=<value>,]                                                                                                   \n" \
			"  |                  %s^~~~ expected a ',' symbol to follow after the 'addr' attribute value.%s                                                                          \n" \
			"3 |     example:                                                                                                                                                         \n" \
			"4 |         ; ...                                                                                                                                                        \n" \
			"5 |     end                                                                                                                                                              \n" \
			"  |                                                                                                                                                                      \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}
}

static void _parse_label_attr_align(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_keyword_align)
	{
		_log_parser_error(token.location,
			"expected a 'align' keyword as the second attribute, but found '%s' token. the attributes list have a specific order that must be followed. follow the example below:\n" \
			"  |                                                                                                                                                                 \n" \
			"2 |     [addr=<value>, %salign%s=<value>, size=<value>, perm=<value>,]                                                                                              \n" \
			"  |                    %s^~~~ expected the 'align' keyword as the second attribute in the attributes list.%s                                                        \n" \
			"3 |     example:                                                                                                                                                    \n" \
			"4 |         ; ...                                                                                                                                                   \n" \
			"5 |     end                                                                                                                                                         \n" \
			"  |                                                                                                                                                                 \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_equal)
	{
		_log_parser_error(token.location,
			"expected a '=' symbol token after 'align' keyword, but found '%s' token. each attribute in the list of attributes must have a value assigned to it. follow the example below:\n" \
			"  |                                                                                                                                                                         \n" \
			"2 |     [addr=<value>, align%s=%s<value>, size=<value>, perm=<value>,]                                                                                                      \n" \
			"  |                         %s^~~~ expected a '=' symbol to follow after the 'align' keyword.%s                                                                             \n" \
			"3 |     example:                                                                                                                                                            \n" \
			"4 |         ; ...                                                                                                                                                           \n" \
			"5 |     end                                                                                                                                                                 \n" \
			"  |                                                                                                                                                                         \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	(void)lasm_lexer_lex(&parser->lexer, &token);

	if ((token.type != lasm_token_type_keyword_auto) && (token.type != lasm_token_type_literal_uval))
	{
		_log_parser_error(token.location,
			"expected an 'auto' keyword or a numeric value for the 'align' attribute, but found '%s' token.",
			lasm_token_type_to_string(token.type)
		);
	}

	if (lasm_token_type_literal_uval == token.type)
	{
		if (token.as.uval > 8)
		{
			_log_parser_error(token.location,
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
		_log_parser_error(token.location,
			"expected a ',' symbolic token after the 'align' attribute's value, but found '%s' token. attributes must have a trailing ',' after their values. follow the example below:\n" \
			"  |                                                                                                                                                                       \n" \
			"2 |     [addr=<value>, align=<value>%s,%s size=<value>, perm=<value>,]                                                                                                    \n" \
			"  |                                 %s^~~~ expected a ',' symbol to follow after the 'align' attribute value.%s                                                           \n" \
			"3 |     example:                                                                                                                                                          \n" \
			"4 |         ; ...                                                                                                                                                         \n" \
			"5 |     end                                                                                                                                                               \n" \
			"  |                                                                                                                                                                       \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}
}

static void _parse_label_attr_size(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_keyword_size)
	{
		_log_parser_error(token.location,
			"expected a 'size' keyword as the third attribute, but found '%s' token. the attributes list have a specific order that must be followed. follow the example below:\n" \
			"  |                                                                                                                                                               \n" \
			"2 |     [addr=<value>, align=<value>, %ssize%s=<value>, perm=<value>,]                                                                                            \n" \
			"  |                                   %s^~~~ expected the 'size' keyword as the third attribute in the attributes list.%s                                         \n" \
			"3 |     example:                                                                                                                                                  \n" \
			"4 |         ; ...                                                                                                                                                 \n" \
			"5 |     end                                                                                                                                                       \n" \
			"  |                                                                                                                                                               \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_equal)
	{
		_log_parser_error(token.location,
			"expected a '=' symbol token after 'size' keyword, but found '%s' token. each attribute in the list of attributes must have a value assigned to it. follow the example below:\n" \
			"  |                                                                                                                                                                         \n" \
			"2 |     [addr=<value>, align=<value>, size%s=%s<value>, perm=<value>,]                                                                                                      \n" \
			"  |                                       %s^~~~ expected a '=' symbol to follow after the 'size' keyword.%s                                                                \n" \
			"3 |     example:                                                                                                                                                            \n" \
			"4 |         ; ...                                                                                                                                                           \n" \
			"5 |     end                                                                                                                                                                 \n" \
			"  |                                                                                                                                                                         \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	(void)lasm_lexer_lex(&parser->lexer, &token);

	if ((token.type != lasm_token_type_keyword_auto) && (token.type != lasm_token_type_literal_uval))
	{
		_log_parser_error(token.location,
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
		_log_parser_error(token.location,
			"expected a ',' symbolic token after the 'size' attribute's value, but found '%s' token. attributes must have a trailing ',' after their values. follow the example below:\n" \
			"  |                                                                                                                                                                      \n" \
			"2 |     [addr=<value>, align=<value>, size=<value>%s,%s perm=<value>,]                                                                                                   \n" \
			"  |                                               %s^~~~ expected a ',' symbol to follow after the 'size' attribute value.%s                                             \n" \
			"3 |     example:                                                                                                                                                         \n" \
			"4 |         ; ...                                                                                                                                                        \n" \
			"5 |     end                                                                                                                                                              \n" \
			"  |                                                                                                                                                                      \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}
}

static void _parse_label_attr_perm(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_keyword_perm)
	{
		_log_parser_error(token.location,
			"expected a 'perm' keyword as the third attribute, but found '%s' token. the attributes list have a specific order that must be followed. follow the example below:\n" \
			"  |                                                                                                                                                               \n" \
			"2 |     [addr=<value>, align=<value>, size=<value>, %sperm%s=<value>,]                                                                                            \n" \
			"  |                                                 %s^~~~ expected the 'perm' keyword as the fourth attribute in the attributes list.%s                          \n" \
			"3 |     example:                                                                                                                                                  \n" \
			"4 |         ; ...                                                                                                                                                 \n" \
			"5 |     end                                                                                                                                                       \n" \
			"  |                                                                                                                                                               \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_equal)
	{
		_log_parser_error(token.location,
			"expected a '=' symbol token after 'size' keyword, but found '%s' token. each attribute in the list of attributes must have a value assigned to it. follow the example below:\n" \
			"  |                                                                                                                                                                         \n" \
			"2 |     [addr=<value>, align=<value>, size=<value>, perm%s=%s<value>,]                                                                                                      \n" \
			"  |                                                     %s^~~~ expected a '=' symbol to follow after the 'perm' keyword.%s                                                  \n" \
			"3 |     example:                                                                                                                                                            \n" \
			"4 |         ; ...                                                                                                                                                           \n" \
			"5 |     end                                                                                                                                                                 \n" \
			"  |                                                                                                                                                                         \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	lasm_ast_perm_type_e perm_type = lasm_ast_perm_type_none;
	(void)lasm_lexer_lex(&parser->lexer, &token);

	switch (token.type)
	{
		case lasm_token_type_keyword_r:    { perm_type = lasm_ast_perm_type_r;    } break;
		case lasm_token_type_keyword_rw:   { perm_type = lasm_ast_perm_type_rw;   } break;
		case lasm_token_type_keyword_rx:   { perm_type = lasm_ast_perm_type_rx;   } break;
		case lasm_token_type_keyword_rwx:  { perm_type = lasm_ast_perm_type_rwx;  } break;
		case lasm_token_type_keyword_auto: { perm_type = lasm_ast_perm_type_none; } break;

		default:
		{
			_log_parser_error(token.location,
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
			.value = perm_type,
		},
	};

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_comma)
	{
		_log_parser_note(token.location,
			"even thought it is not enforced by an error, it is a standard and a good practice to trail each attribute with a comma. follow the example below:\n" \
			"  |                                                                                                                                              \n" \
			"2 |     [addr=<value>, align=<value>, size=<value>, perm=<value>%s,%s]                                                                           \n" \
			"  |                                                             %s^~~~ expected a trailing comma here.%s                                         \n" \
			"3 |     example:                                                                                                                                 \n" \
			"4 |         ; ...                                                                                                                                \n" \
			"5 |     end                                                                                                                                      \n" \
			"  |                                                                                                                                              \n",
			lasm_cyan, lasm_reset,
			lasm_cyan, lasm_reset
		);

		lasm_lexer_unlex(&parser->lexer, &token);
	}
}

static bool_t _parse_label_header(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	lasm_token_s token = lasm_token_new(lasm_token_type_none, parser->lexer.location);
	(void)lasm_lexer_lex(&parser->lexer, &token);

	if (lasm_lexer_should_stop(token.type))
	{
		return false;
	}

	label->location = parser->lexer.location;

	if (token.type != lasm_token_type_symbolic_left_bracket)
	{
		_log_parser_error(token.location,
			"expected a symbolic token '[', but found '%s' token. all global definitions must be labels which start with the attributes list. follow the example below:\n" \
			"  |                                                                                                                                                       \n" \
			"2 |     %s[%saddr=<value>, align=<value>, size=<value>, perm=<value>,]                                                                                    \n" \
			"  |     %s^~~~ expected a '[' bracket to start the attributes list for a label in the global scope.%s                                                     \n" \
			"3 |     example:                                                                                                                                          \n" \
			"4 |         ; ...                                                                                                                                         \n" \
			"5 |     end                                                                                                                                               \n" \
			"  |                                                                                                                                                       \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	_parse_label_attr_addr( parser, label);
	_parse_label_attr_align(parser, label);
	_parse_label_attr_size( parser, label);
	_parse_label_attr_perm( parser, label);

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_right_bracket)
	{
		_log_parser_error(token.location,
			"expected a symbolic token ']' after the attributes list, but found '%s' token. an attributes list must be closed with the ']' symbolic token. follow the example below:\n" \
			"  |                                                                                                                                                                    \n" \
			"2 |     [addr=<value>, align=<value>, size=<value>, perm=<value>,%s]%s                                                                                                 \n" \
			"  |                                                              %s^~~~ expected a ']' bracket to close the attributes list for a label in the global scope.%s         \n" \
			"3 |     example:                                                                                                                                                       \n" \
			"4 |         ; ...                                                                                                                                                      \n" \
			"5 |     end                                                                                                                                                            \n" \
			"  |                                                                                                                                                                    \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_ident)
	{
		_log_parser_error(token.location,
			"expected an identifier token after attributes list for the label, but found '%s' token. a label name must follow the attributes list. follow the example below:\n" \
			"  |                                                                                                                                                            \n" \
			"2 |     [addr=<value>, align=<value>, size=<value>, perm=<value>,]                                                                                             \n" \
			"3 |     %sexample%s:                                                                                                                                           \n" \
			"  |     %s^~~~ expected an identifier for a label to follow after the attributes list.%s                                                                       \n" \
			"4 |         ; ...                                                                                                                                              \n" \
			"5 |     end                                                                                                                                                    \n" \
			"  |                                                                                                                                                            \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	label->name = token.as.ident.data;

	if (lasm_lexer_lex(&parser->lexer, &token) != lasm_token_type_symbolic_colon)
	{
		_log_parser_error(token.location,
			"expected a ':' symbolic token after the label's identifier token, but found '%s' token. a ':' symbolic token must follow the label's identifier token. follow the example below:\n" \
			"  |                                                                                                                                                                             \n" \
			"2 |     [addr=<value>, align=<value>, size=<value>, perm=<value>,]                                                                                                              \n" \
			"3 |     example%s:%s                                                                                                                                                            \n" \
			"  |            %s^~~~ expected a ':' to follow after an identifier of a label.%s                                                                                                \n" \
			"4 |         ; ...                                                                                                                                                               \n" \
			"5 |     end                                                                                                                                                                     \n" \
			"  |                                                                                                                                                                             \n",
			lasm_token_type_to_string(token.type),
			lasm_red, lasm_reset,
			lasm_red, lasm_reset
		);
	}

	label->body_tokens = lasm_tokens_vector_new(parser->arena, 1);

	while (!lasm_lexer_should_stop(lasm_lexer_lex(&parser->lexer, &token)))
	{
		if (lasm_token_type_keyword_end == token.type)
		{
			break;
		}

		lasm_tokens_vector_push(&label->body_tokens, token);
	}

	return true;
}

static void _parse_label_body(lasm_parser_s* const parser, lasm_ast_label_s* const label)
{
	lasm_debug_assert(parser != NULL);
	lasm_debug_assert(label != NULL);

	label->body = lasm_bytes_vector_new(parser->arena, 1);

	switch (parser->config->arch)
	{
		case lasm_arch_type_z80:
		{
			// todo: parse the z80 assembly!
			z80_parser_parse_tokens(&parser->lexer, &parser->labels, label);
		} break;

		case lasm_arch_type_rl78:
		{
			// todo: parse the rl78 assembly!
			rl78_parser_parse_tokens(&parser->lexer, &parser->labels, label);
		} break;

		default:
		{
			lasm_debug_assert(0);
		} break;
	}
}
