
/**
 * @file lexer.c
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
#include "lasm/lexer.h"

#include <sys/stat.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#define log_lexer_error(_location, _format, ...)                               \
	do                                                                         \
	{                                                                          \
		(void)fprintf(stderr, "%s:%lu:%lu: ",                                  \
			(_location).file, (_location).line, (_location).column);           \
		lasm_logger_error(_format, ## __VA_ARGS__);                            \
		lasm_common_exit(1);                                                   \
	} while (0)

/**
 * @brief Update location by provided symbol c.
 * 
 * @param location location to update
 * @param c        symbol to reference
 */
static void update_location(lasm_location_s* const location, const utf8char_t c);

/**
 * @brief Append to the lexer's buffer.
 * 
 * @param lexer  lexer to append to
 * @param buffer buffer to append to
 * @param size   length of the buffer
 */
static void append_buffer(lasm_lexer_s* const lexer, const char_t* const buffer, const uint64_t size);

/**
 * @brief Fetch next_utf8char utf-8 symbol from the lexer and update the location.
 * 
 * @param lexer    lexer reference
 * @param location location to update
 * @param buffer   buffer fetched symbol
 * 
 * @return utf8char_t
 */
static utf8char_t next_utf8char(lasm_lexer_s* const lexer, lasm_location_s* const location, const bool_t buffer);

/**
 * @brief Check if symbol is a white space symbol.
 * 
 * @param c symbol to check
 * 
 * @return bool_t
 */
static bool_t is_symbol_a_white_space(const utf8char_t c);

/**
 * @brief Get the utf-8 symbol from lexer's file and update location.
 * 
 * @param lexer    lexer reference
 * @param location location to update
 * 
 * @return utf8char_t
 */
static utf8char_t get_utf8char(lasm_lexer_s* const lexer, lasm_location_s* const location);

/**
 * @brief Clear (reset) the lexer's buffer.
 * 
 * @param lexer lexer reference
 */
static void clear_buffer(lasm_lexer_s* const lexer);

/**
 * @brief Consume the lexer's buffer.
 * 
 * @param lexer  lexer reference
 * @param length amount of the buffer to consume
 */
static void consume_buffer(lasm_lexer_s* const lexer, const uint64_t length);

/**
 * @brief Push utf-8 symbol to lexer's cache.
 * 
 * @param lexer  lexer reference
 * @param c      symbol to push
 * @param buffer buffer the symbol
 */
static void push_utf8char(lasm_lexer_s* const lexer, const utf8char_t c, const bool_t buffer);

/**
 * @brief Check if symbol is first of numeric literal.
 * 
 * @param c symbol to check
 * 
 * @return bool_t
 */
static inline bool_t is_symbol_first_of_numeric_literal(const utf8char_t c);

/**
 * @brief Check if symbol is first of a keyword or identifier.
 * 
 * @param c symbol to check
 * 
 * @return bool_t
 */
static inline bool_t is_symbol_first_of_keyword_or_identifier(const utf8char_t c);

/**
 * @brief Check if symbol is not first of a keyword or identifier.
 * 
 * @param c symbol to check
 * 
 * @return bool_t
 */
static inline bool_t is_symbol_not_first_of_keyword_or_identifier(const utf8char_t c);

/**
 * @brief Skip (nested) multi line comments.
 * 
 * @param lexer lexer reference
 * @param c     start symbol
 * 
 * @return bool_t
 */
static bool_t skip_nested_multi_line_comments(lasm_lexer_s* const lexer, utf8char_t c);

/**
 * @brief Compute exponent for a numeric literal.
 * 
 * @param value     value that has an exponent
 * @param exponent  exponent value
 * @param is_signed is signed
 * 
 * @return uint64_t
 */
static uint64_t compute_numeric_literal_exponent(uint64_t value, const uint64_t exponent, const bool_t is_signed);

/**
 * @brief Lex keyword or identifier token.
 * 
 * @param lexer lexer reference
 * @param token token reference
 * 
 * @return lasm_token_type_e
 */
static lasm_token_type_e lex_keyword_or_identifier(lasm_lexer_s* const lexer, lasm_token_s* const token);

/**
 * @brief Lex numeric literal.
 * 
 * @param lexer lexer reference
 * @param token token reference
 * 
 * @return lasm_token_type_e
 */
static lasm_token_type_e lex_numeric_literal_token(lasm_lexer_s* const lexer, lasm_token_s* const token);

/**
 * @brief Decode a rune.
 * 
 * @param lexer lexer rederence
 * @param out   result buffer
 * 
 * @return uint8_t
 */
static uint8_t decode_single_rune(lasm_lexer_s* const lexer, char_t* const out);

/**
 * @brief Lex rune literal token.
 * 
 * @param lexer lexer reference
 * @param token token reference
 * 
 * @return lasm_token_type_e
 */
static lasm_token_type_e lex_rune_literal_token(lasm_lexer_s* const lexer, lasm_token_s* const token);

/**
 * @brief Lex single line string literal token.
 * 
 * @param lexer lexer reference
 * @param token token reference
 * 
 * @return lasm_token_type_e
 */
static lasm_token_type_e lex_single_line_string_literal_token(lasm_lexer_s* const lexer, lasm_token_s* const token);

/**
 * @brief Lex 2 symbols token.
 * 
 * @param lexer lexer reference
 * @param token token reference
 * @param c     symbol to start lexing with
 * 
 * @return lasm_token_type_e 
 */
static lasm_token_type_e lex_2_symbols_token(lasm_lexer_s* const lexer, lasm_token_s* const token, utf8char_t c);

lasm_lexer_s lasm_lexer_new(lasm_arena_s* const arena, const char_t* const file_path)
{
	lasm_debug_assert(arena != NULL);
	lasm_debug_assert(file_path != NULL);

	typedef struct stat stats_s;
	stats_s stats = {0};

	if (stat(file_path, &stats) != 0)
	{
		switch (errno)
		{
			case ENOENT:
			{
				lasm_logger_error("unable to open %s for reading -- file not found.", file_path);
				lasm_common_exit(1);
			} break;

			case EACCES:
			{
				lasm_logger_error("unable to open %s for reading -- permission denied.", file_path);
				lasm_common_exit(1);
			} break;

			case ENAMETOOLONG:
			{
				lasm_logger_error("unable to open %s for reading -- path name exceeds the system-defined maximum length.", file_path);
				lasm_common_exit(1);
			} break;

			default:
			{
				lasm_logger_error("unable to open %s for reading -- failed to stat.", file_path);
				lasm_common_exit(1);
			} break;
		}
	}

	if (S_ISDIR(stats.st_mode))
	{
		lasm_logger_error("unable to open %s for reading -- it is a directory.", file_path);
		lasm_common_exit(1);
	}

	FILE* const file = fopen(file_path, "rt");

	if (NULL == file)
	{
		lasm_logger_error("unable to open %s for reading -- failed to open.", file_path);
		lasm_common_exit(1);
	}

	const uint64_t buffer_capacity = 256;
	char_t* const buffer = lasm_arena_alloc(arena, buffer_capacity * sizeof(char_t));
	lasm_debug_assert(buffer != NULL);

	return (lasm_lexer_s)
	{
		.arena = arena,
		.file  = file,
		.token = (lasm_token_s)
		{
			.type = lasm_token_type_none,
		},
		.location = (lasm_location_s)
		{
			.file   = file_path,
			.line   = 1,
			.column = 0,
		},
		.cache =
		{
			[0] = lasm_utf8_invalid,
			[1] = lasm_utf8_invalid,
		},
		.buffer =
		{
			.capacity = buffer_capacity,
			.data     = buffer,
			.length   = 0,
		},
	};
}

void lasm_lexer_drop(lasm_lexer_s* const lexer)
{
	lasm_debug_assert(lexer != NULL);
	(void)fclose(lexer->file);
}

lasm_token_type_e lasm_lexer_lex(lasm_lexer_s* const lexer, lasm_token_s* const token)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(token != NULL);

	if (lexer->token.type != lasm_token_type_none)
	{
		*token = lexer->token;
		lexer->token.type = lasm_token_type_none;
		return token->type;
	}

	utf8char_t c = get_utf8char(lexer, &token->location);

	if (lasm_utf8_invalid == c)
	{
		*token = lasm_token_new(lasm_token_type_eof, lexer->location);
		lexer->token = lasm_token_new(lasm_token_type_none, lexer->location);
		return token->type;
	}

	if (is_symbol_first_of_numeric_literal(c))
	{
		push_utf8char(lexer, c, false);
		return lex_numeric_literal_token(lexer, token);
	}

	if (is_symbol_first_of_keyword_or_identifier(c))
	{
		push_utf8char(lexer, c, false);
		return lex_keyword_or_identifier(lexer, token);
	}

	const lasm_location_s start_location = lexer->location;

	switch (c)
	{
		case ';':
		case '#':
		{
			while (((c = next_utf8char(lexer, NULL, true)) != lasm_utf8_invalid) && (c != '\n'));
			clear_buffer(lexer);
			return lasm_lexer_lex(lexer, token);
		} break;
		case '/':  { (void)lex_2_symbols_token(lexer, token, c);                                      } break;

		case '\'': { (void)lex_rune_literal_token(lexer, token);                                      } break;
		case '\"': { (void)lex_single_line_string_literal_token(lexer, token);                        } break;

		case '.':  { *token = lasm_token_new(lasm_token_type_symbolic_dot,           start_location); } break;
		case ',':  { *token = lasm_token_new(lasm_token_type_symbolic_comma,         start_location); } break;
		case '=':  { *token = lasm_token_new(lasm_token_type_symbolic_equal,         start_location); } break;
		case ':':  { *token = lasm_token_new(lasm_token_type_symbolic_colon,         start_location); } break;
		case '[':  { *token = lasm_token_new(lasm_token_type_symbolic_left_bracket,  start_location); } break;
		case ']':  { *token = lasm_token_new(lasm_token_type_symbolic_right_bracket, start_location); } break;

		default:
		{
			static char_t buffer[lasm_utf8_max_size];
			const uint8_t size = lasm_utf8_encode(buffer, c);
			log_lexer_error(lexer->location, "invalid token encountered: '%.*s'", (int32_t)size, buffer);
		} break;
	}

	return token->type;
}

bool_t lasm_lexer_should_stop(const lasm_token_type_e type)
{
	return ((lasm_token_type_none == type) || (lasm_token_type_eof == type));
}

void lasm_lexer_unlex(lasm_lexer_s* const lexer, const lasm_token_s* const token)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(token != NULL);
	lasm_debug_assert(token->type != lasm_token_type_none);
	lexer->token = *token;
}

static void update_location(lasm_location_s* const location, const utf8char_t c)
{
	lasm_debug_assert(location != NULL);

	if ('\n' == c)
	{
		location->line++;
		location->column = 0;
	}
	else
	{
		location->column++;
	}
}

static void append_buffer(lasm_lexer_s* const lexer, const char_t* const buffer, const uint64_t size)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(buffer != NULL);
	lasm_debug_assert(size > 0);

	if ((lexer->buffer.length + size) >= lexer->buffer.capacity)
	{
		lexer->buffer.capacity *= 2;
		char_t* const data = lasm_arena_alloc(lexer->arena, lexer->buffer.capacity);
		lasm_debug_assert(data != NULL);

		for (uint64_t index = 0; index < lexer->buffer.length; ++index) data[index] = lexer->buffer.data[index];
		lexer->buffer.data = data;
	}

	lasm_common_memcpy(lexer->buffer.data + lexer->buffer.length, buffer, size);
	lexer->buffer.length += size;
	lexer->buffer.data[lexer->buffer.length] = 0;
}

static utf8char_t next_utf8char(lasm_lexer_s* const lexer, lasm_location_s* const location, const bool_t buffer)
{
	lasm_debug_assert(lexer != NULL);
	utf8char_t c = lasm_utf8_invalid;

	if (lexer->cache[0] != lasm_utf8_invalid)
	{
		c = lexer->cache[0];
		lexer->cache[0] = lexer->cache[1];
		lexer->cache[1] = lasm_utf8_invalid;
	}
	else
	{
		c = lasm_utf8_get(lexer->file);
		update_location(&lexer->location, c);

		if ((lasm_utf8_invalid == c) && (!feof(lexer->file)))
		{
			static char_t utf8_buffer[lasm_utf8_max_size];
			const uint8_t size = lasm_utf8_encode(utf8_buffer, c);
			log_lexer_error(lexer->location, "invalid utf-8 sequence encountered: %.*s", (int32_t)size, utf8_buffer);
		}
	}

	if (location != NULL)
	{
		*location = lexer->location;

		for (uint8_t index = 0; (index < 2) && (lexer->cache[index] != lasm_utf8_invalid); ++index)
		{
			update_location(&lexer->location, lexer->cache[index]);
		}
	}

	if ((lasm_utf8_invalid == c) || (!buffer))
	{
		return c;
	}

	static char_t utf8_buffer[lasm_utf8_max_size];
	const uint8_t size = lasm_utf8_encode(utf8_buffer, c);
	append_buffer(lexer, utf8_buffer, size);
	return c;
}

static bool_t is_symbol_a_white_space(const utf8char_t c)
{
	return (('\t' == c) || ('\n' == c) || ('\r' == c) || (' ' == c));
}

static utf8char_t get_utf8char(lasm_lexer_s* const lexer, lasm_location_s* const location)
{
	lasm_debug_assert(lexer != NULL);
	utf8char_t c = lasm_utf8_invalid;
	while (((c = next_utf8char(lexer, location, false)) != lasm_utf8_invalid) && is_symbol_a_white_space(c));
	return c;
}

static void clear_buffer(lasm_lexer_s* const lexer)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(lexer->buffer.data != NULL);
	lexer->buffer.length = 0;
	lexer->buffer.data[0] = 0;
}

static void consume_buffer(lasm_lexer_s* const lexer, const uint64_t length)
{
	for (uint64_t index = 0; index < length; ++index)
	{
		while (0x80 == (lexer->buffer.data[--lexer->buffer.length] & 0xC0));
	}

	lexer->buffer.data[lexer->buffer.length] = 0;
}

static void push_utf8char(lasm_lexer_s* const lexer, const utf8char_t c, const bool_t buffer)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(lasm_utf8_invalid == lexer->cache[1]);
	lexer->cache[1] = lexer->cache[0];
	lexer->cache[0] = c;

	if (buffer)
	{
		consume_buffer(lexer, 1);
	}
}

static inline bool_t is_symbol_first_of_numeric_literal(const utf8char_t c)
{
	return ((c <= 0x7F) && isdigit(c));
}

static inline bool_t is_symbol_first_of_keyword_or_identifier(const utf8char_t c)
{
	return ((c <= 0x7F) && (isalpha(c) || ('_' == c)));
}

static inline bool_t is_symbol_not_first_of_keyword_or_identifier(const utf8char_t c)
{
	return ((c <= 0x7F) && (isalnum(c) || ('_' == c) || ('-' == c)));
}

static bool_t skip_nested_multi_line_comments(lasm_lexer_s* const lexer, utf8char_t c)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(c != lasm_utf8_invalid);
	
	utf8char_t last_c = c; c = next_utf8char(lexer, NULL, true);
	while (((last_c = c, c = next_utf8char(lexer, NULL, true)) != lasm_utf8_invalid) && ((last_c != '*') || (c != '/')))
	{
		if (('/' == last_c) && ('*' == c))
		{
			if (!skip_nested_multi_line_comments(lexer, c))
			{
				return false;
			}
		}
	}

	return (c != lasm_utf8_invalid);
}

static uint64_t compute_numeric_literal_exponent(uint64_t value, const uint64_t exponent, const bool_t is_signed)
{
	if (value == 0)
	{
		return 0;
	}

	for (uint64_t index = 0; index < exponent; ++index)
	{
		uint64_t old_value = value;
		value *= 10;

		if (value / 10 != old_value)
		{
			errno = ERANGE;
			return (uint64_t)INT64_MAX;
		}
	}

	if (is_signed && (value > (uint64_t)INT64_MIN))
	{
		errno = ERANGE;
		return (uint64_t)INT64_MAX;
	}

	return value;
}

static lasm_token_type_e lex_keyword_or_identifier(lasm_lexer_s* const lexer, lasm_token_s* const token)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(token != NULL);

	token->location = lexer->location;
	utf8char_t c = next_utf8char(lexer, &token->location, true);
	lasm_debug_assert(is_symbol_first_of_keyword_or_identifier(c));

	while ((c = next_utf8char(lexer, NULL, true)) != lasm_utf8_invalid)
	{
		if (!is_symbol_not_first_of_keyword_or_identifier(c))
		{
			push_utf8char(lexer, c, true);
			break;
		}
	}

	for (uint64_t index = 0; index < lasm_token_type_keywords_count; ++index)
	{
		if (lasm_common_strcmp(lasm_token_type_to_string(index), lexer->buffer.data) == 0)
		{
			clear_buffer(lexer);
			token->type = index;
			return token->type;
		}
	}

	token->type = lasm_token_type_ident;
	token->as.ident.data = lasm_arena_alloc(lexer->arena, lexer->buffer.length);
	lasm_debug_assert(token->as.ident.data != NULL);

	for (uint64_t index = 0; index < lexer->buffer.length; ++index) token->as.ident.data[index] = lexer->buffer.data[index];
	token->as.ident.length = lexer->buffer.length;
	clear_buffer(lexer);
	return token->type;
}

static lasm_token_type_e lex_numeric_literal_token(lasm_lexer_s* const lexer, lasm_token_s* const token)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(token != NULL);

	// enum { flag_flt = 3, flag_exp, flag_suff, flag_dig, };
	enum { base_bin = 1, base_oct, base_hex, base_dec = 0x07, base_mask = base_dec, };
	_Static_assert((base_bin | base_oct | base_hex | base_dec) == base_dec,
		"base_dec bits must be a superset of all other bases"
	);

	static const char_t formats_chars[][24] =
	{
		[base_bin] = "01",
		[base_oct] = "01234567",
		[base_dec] = "0123456789",
		[base_hex] = "0123456789abcdefABCDEF",
	};

	int32_t state = base_dec;
	int32_t old_state = base_dec;
	int32_t base = 10;

	utf8char_t c = next_utf8char(lexer, &token->location, true), last = 0;
	lasm_debug_assert(is_symbol_first_of_numeric_literal(c));

	if ('0' == c)
	{
		c = next_utf8char(lexer, NULL, true);

		if (is_symbol_first_of_numeric_literal(c))
		{
			log_lexer_error(token->location, "leading zero in base 10 literal.");
		}
		else if ('b' == c)
		{
			state = base_bin;
			base = 2;
		}
		else if ('o' == c)
		{
			state = base_oct;
			base = 8;
		}
		else if ('x' == c)
		{
			state = base_hex;
			base = 16;
		}
	}

	if (state != base_dec)
	{
		last = c;
		c = next_utf8char(lexer, NULL, true);
	}

	uint64_t exponent_start = 0;

	do
	{
		if (lasm_common_strchr(formats_chars[state & base_mask], (int32_t)c))
		{
			last = c;
			continue;
		}
		else if (c > 0x7f)
		{
			goto end;
		}

		old_state = state;

		switch (c)
		{
			default:
			{
				goto end;
			} break;
		}

		last = c;
	} while ((c = next_utf8char(lexer, NULL, true)) != lasm_utf8_invalid);

	last = 0;

end:
	if (last && (!lasm_common_strchr("su", (int32_t)last)) && (!lasm_common_strchr(formats_chars[state & base_mask], (int32_t)last)))
	{
		state = old_state;
		push_utf8char(lexer, c, true);
		push_utf8char(lexer, last, true);
	}
	else if (c != lasm_utf8_invalid)
	{
		push_utf8char(lexer, c, true);
	}

	typedef enum { kind_unknown = -1, kind_iconst, kind_unsigned, } kind_e;
	kind_e kind = kind_unknown;

	if (kind_unknown == kind)
	{
		kind = kind_iconst;
		token->type = lasm_token_type_literal_uval;
	}

	uint64_t exponent = 0;
	errno = 0;

	if (exponent_start)
	{
		exponent = (uint64_t)strtoumax(lexer->buffer.data + exponent_start + 1, NULL, 10);
	}

	const uint8_t prefix_offset = ((10 == base) ? 0 : 2);
	token->as.uval = strtoumax(lexer->buffer.data + prefix_offset, NULL, base);
	token->as.uval = compute_numeric_literal_exponent(token->as.uval, exponent, false);

	if (errno == ERANGE)
	{
		log_lexer_error(token->location, "integer literal overflow.");
	}

	if ((kind_iconst == kind) && (token->as.uval > (uint64_t)INT64_MAX))
	{
		token->type = lasm_token_type_literal_uval;
	}

	clear_buffer(lexer);
	return token->type;
}

static uint8_t decode_single_rune(lasm_lexer_s* const lexer, char_t* const out)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(out != NULL);

	utf8char_t c = next_utf8char(lexer, NULL, false);
	lasm_debug_assert(c != lasm_utf8_invalid);

	switch (c)
	{
		case '\\':
		{
			const lasm_location_s location = lexer->location;
			c = next_utf8char(lexer, NULL, false);

			char_t buffer[9];
			char_t* end_pointer = NULL;

			switch (c)
			{
				case '0':  { out[0] = '\0'; return 1; } break;
				case 'a':  { out[0] = '\a'; return 1; } break;
				case 'b':  { out[0] = '\b'; return 1; } break;
				case 'f':  { out[0] = '\f'; return 1; } break;
				case 'n':  { out[0] = '\n'; return 1; } break;
				case 'r':  { out[0] = '\r'; return 1; } break;
				case 't':  { out[0] = '\t'; return 1; } break;
				case 'v':  { out[0] = '\v'; return 1; } break;
				case '\\': { out[0] = '\\'; return 1; } break;
				case '\'': { out[0] = '\''; return 1; } break;
				case '"':  { out[0] = '\"'; return 1; } break;

				case 'x':
				{
					buffer[0] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[1] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[2] = '\0';

					c = (utf8char_t)strtoul(buffer, &end_pointer, 16);

					if (*end_pointer != '\0')
					{
						log_lexer_error(location, "invalid hex literal");
					}

					out[0] = (char_t)c;
					return 1;
				} break;

				case 'u':
				{
					buffer[0] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[1] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[2] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[3] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[4] = '\0';

					c = (utf8char_t)strtoul(buffer, &end_pointer, 16);

					if (*end_pointer != '\0')
					{
						log_lexer_error(location, "invalid hex literal");
					}

					return lasm_utf8_encode(out, c);
				} break;

				case 'U':
				{
					buffer[0] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[1] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[2] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[3] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[4] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[5] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[6] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[7] = (char_t)next_utf8char(lexer, NULL, false);
					buffer[8] = '\0';

					c = (utf8char_t)strtoul(&buffer[0], &end_pointer, 16);

					if (*end_pointer != '\0')
					{
						log_lexer_error(location, "invalid hex literal");
					}

					return lasm_utf8_encode(out, c);
				} break;

				case lasm_utf8_invalid:
				{
					log_lexer_error(lexer->location, "unexpected end of file");
					return 0;
				} break;

				default:
				{
					log_lexer_error(location, "invalid escape sequence '\\%c'", c);
					return 0;
				} break;
			}

			log_lexer_error(location, "invalid escape sequence '\\%c'", c);
			return 0;
		} break;

		default:
		{
			return lasm_utf8_encode(out, c);
		} break;
	}
}

static lasm_token_type_e lex_rune_literal_token(lasm_lexer_s* const lexer, lasm_token_s* const token)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(token != NULL);

	token->location = (lasm_location_s)
	{
		.file   = lexer->location.file,
		.line   = lexer->location.line,
		.column = lexer->location.column - 1,
	};

	utf8char_t c = next_utf8char(lexer, &lexer->location, false);

	switch (c)
	{
		case '\'':
		{
			log_lexer_error(token->location, "expected rune before trailing single quote in rune literal");
		} break;

		case '\\':
		{
			char_t buffer[lasm_utf8_max_size + 1];

			push_utf8char(lexer, c, false);
			(void)decode_single_rune(lexer, buffer);

			const char_t* string = buffer;
			token->as.rune = lasm_utf8_decode(&string);

			if (lasm_utf8_invalid == token->as.rune)
			{
				log_lexer_error(token->location, "invalid utf-8 sequence in rune literal");
			}
		} break;

				    //  _________ note: all of these symbols are supported as escape sequences.
		case '\0':  // /////////
		case '\a':  // ////////
		case '\b':  // ///////
		case '\f':  // //////
		case '\n':  // /////
		case '\r':  // ////
		case '\t':  // ///
		case '\v':  // //
		case '\"':  // /
		{
			log_lexer_error(token->location, "invalid rune literal encountered: '%c'!", (char_t)c);
		} break;

		default:
		{
			token->as.rune = c;
		} break;
	}

	if (next_utf8char(lexer, NULL, false) != '\'')
	{
		log_lexer_error(token->location, "expected trailing single quote in rune literal");
	}

	token->type = lasm_token_type_literal_rune;
	return token->type;
}

static lasm_token_type_e lex_single_line_string_literal_token(lasm_lexer_s* const lexer, lasm_token_s* const token)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(token != NULL);

	token->location = (lasm_location_s)
	{
		.file   = lexer->location.file,
		.line   = lexer->location.line,
		.column = lexer->location.column - 1,
	};

	utf8char_t c;
	while (((c = next_utf8char(lexer, NULL, false)) != lasm_utf8_invalid) && (c != '\"'))
	{
		switch (c)
		{
				    //  _________ note: all of these symbols are supported as escape sequences.
			case '\0':  // /////////
			case '\a':  // ////////
			case '\b':  // ///////
			case '\f':  // //////
			case '\n':  // /////
			case '\r':  // ////
			case '\t':  // ///
			case '\v':  // //
			case '\"':  // /
			{
				log_lexer_error(token->location, "invalid rune encountered in single line string literal: '%c'!", (char_t)c);
			} break;
		}

		char_t buffer[lasm_utf8_max_size];
		push_utf8char(lexer, c, false);

		const uint8_t size = decode_single_rune(lexer, buffer);
		append_buffer(lexer, buffer, size);
	}

	if (c == lasm_utf8_invalid)
	{
		log_lexer_error(token->location, "unclosed single line string literal found!");
	}

	lasm_debug_assert(lexer->buffer.length > 0);
	char_t* const data = (char_t* const)lasm_arena_alloc(lexer->arena, lexer->buffer.length);
	lasm_debug_assert(data != NULL);

	lasm_common_memcpy(data, lexer->buffer.data, lexer->buffer.length);
	token->type = lasm_token_type_literal_str;
	token->as.str.length = lexer->buffer.length;
	token->as.str.data = data;

	clear_buffer(lexer);
	return token->type;
}

static lasm_token_type_e lex_2_symbols_token(lasm_lexer_s* const lexer, lasm_token_s* const token, utf8char_t c)
{
	lasm_debug_assert(lexer != NULL);
	lasm_debug_assert(token != NULL);
	lasm_debug_assert(c != lasm_utf8_invalid);

	token->location = lexer->location;

	switch (c)
	{
		case '/':
		{
			switch ((c = next_utf8char(lexer, NULL, false)))
			{
				case '/':
				{
					while (((c = next_utf8char(lexer, NULL, true)) != lasm_utf8_invalid) && (c != '\n'));
					clear_buffer(lexer);
					return lasm_lexer_lex(lexer, token);
				} break;

				case '*':
				{
					if (!skip_nested_multi_line_comments(lexer, c))
					{
						log_lexer_error(token->location, "unclosed multi line comment found!");
					}

					clear_buffer(lexer);
					return lasm_lexer_lex(lexer, token);
				} break;
			}
		} break;

		default:
		{
			// note: should never ever happen as this function will get symbols
			// that are already verified to be correct ones!
			lasm_debug_assert(0);  // note: sanity check for developers.
		} break;
	}

	return token->type;
}
