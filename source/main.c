
/**
 * @file main.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#include "main.h"

#include "lasm/version.h"
#include "lasm/debug.h"
#include "lasm/logger.h"
#include "lasm/arena.h"
#include "lasm/token.h"
#include "lasm/lexer.h"

static const char_t* g_program = NULL;
static const char_t* const g_usage_banner =
	"usage: %s <command>\n"
	"\n"
	"commands:\n"
	"\n"
	"    build [options] <source.lasm>       build the project with provided source file.\n"
	"        required:\n"
	"            <source.lasm>               source file to build.\n"
	"        optional:\n"
	"            -e, --entry <name>          set the entry name symbol for the executable. defaults to the\n"
	"                                        name \'main\'.\n"
	"            -o, --output <path>         set the output path for the executable. defaults to the name of\n"
	"                                        provided source file with extension removed if not provided.\n"
	"\n"
	"    help                                print this help message banner.\n"
	"    version                             print the version of this executable.\n"
	"\n"
	"notice:\n"
	"    this executable is distributed under the \"lasm gplv1\" license.\n";

static void print_usage(void);

static const char_t* shift_args(int32_t* const argc, const char_t*** const argv);

static bool_t match_option(const char_t* const option, const char_t* const long_name, const char_t* const short_name);

static const char_t* get_option_argument(const char_t* const option, int32_t* const argc, const char_t*** const argv);

static void build(int32_t* const argc, const char_t*** const argv);

int32_t main(int32_t argc, const char_t** argv)
{
	lasm_debug_assert(argc > 0);
	lasm_debug_assert(argv != NULL);

	lasm_debug_assert(NULL == g_program);
	g_program = shift_args(&argc, &argv);
	lasm_debug_assert(g_program != NULL);

	if (argc <= 0)
	{
		lasm_logger_error("no command was provided.");
		print_usage();
		lasm_common_exit(1);
	}

	const char_t* const command = shift_args(&argc, &argv);
	lasm_debug_assert(command != NULL);

	if (lasm_common_strcmp(command, "build") == 0)
	{
		build(&argc, &argv);
	}
	else if (lasm_common_strcmp(command, "help") == 0)
	{
		print_usage();
		lasm_common_exit(0);
	}
	else if (lasm_common_strcmp(command, "version") == 0)
	{
		lasm_logger_log("%s %s", g_program, lasm_version);
		lasm_common_exit(0);
	}
	else
	{
		lasm_logger_error("unknown or invalid command was provided: %s.", command);
		print_usage();
		lasm_common_exit(1);
	}

	return 0;
}

static void print_usage(void)
{
	lasm_debug_assert(g_usage_banner != NULL);
	lasm_debug_assert(g_program != NULL);
	lasm_logger_log(g_usage_banner, g_program);
}

static const char_t* shift_args(int32_t* const argc, const char_t*** const argv)
{
	lasm_debug_assert(argc != NULL);
	lasm_debug_assert(argv != NULL);

	const char_t* argument = NULL;

	if (*argc > 0)
	{
		argument = **argv;
		++(*argv);
		--(*argc);
	}

	return argument;
}

static bool_t match_option(const char_t* const option, const char_t* const long_name, const char_t* const short_name)
{
	lasm_debug_assert(option != NULL);
	lasm_debug_assert(long_name != NULL);
	lasm_debug_assert(short_name != NULL);

	const uint64_t option_length     = lasm_common_strlen(option);
	const uint64_t long_name_length  = lasm_common_strlen(long_name);
	const uint64_t short_name_length = lasm_common_strlen(short_name);

	return (
		((option_length == long_name_length ) && (lasm_common_strncmp(option, long_name, option_length ) == 0)) ||
		((option_length == short_name_length) && (lasm_common_strncmp(option, short_name, option_length) == 0))
	);
}

static const char_t* get_option_argument(const char_t* const option, int32_t* const argc, const char_t*** const argv)
{
	lasm_debug_assert(option != NULL);
	lasm_debug_assert(argc != NULL);
	lasm_debug_assert(argv != NULL);

	const char_t* const argument = shift_args(argc, argv);

	if (NULL == argument)
	{
		lasm_logger_error("option '%s' requires an argument, but none was provided.", option);
		print_usage();
		lasm_common_exit(1);
	}

	return argument;
}

static void build(int32_t* const argc, const char_t*** const argv)
{
	lasm_debug_assert(argc != NULL);
	lasm_debug_assert(argv != NULL);

	const char_t* entry  = NULL;
	const char_t* output = NULL;
	const char_t* source = NULL;

	for (uint64_t index = 0; true; ++index)
	{
		const char_t* const option = shift_args(argc, argv);
		if (NULL == option) { break; }

		if (match_option(option, "--entry", "-e"))
		{
			if (entry != NULL)
			{
				lasm_logger_error("multiple --entry, -e arguments found in the command line arguments in 'build' command.");
				print_usage();
				lasm_common_exit(1);
			}

			const char_t* const entry_as_string = get_option_argument(option, argc, argv);
			lasm_debug_assert(entry_as_string != NULL);
			entry = entry_as_string;
		}
		else if (match_option(option, "--output", "-o"))
		{
			if (output != NULL)
			{
				lasm_logger_error("multiple --output, -o arguments found in the command line arguments in 'build' command.");
				print_usage();
				lasm_common_exit(1);
			}

			const char_t* const output_as_string = get_option_argument(option, argc, argv);
			lasm_debug_assert(output_as_string != NULL);
			output = output_as_string;
		}
		else
		{
			if (source != NULL)
			{
				lasm_logger_error("multiple source files found in the command line arguments in 'build' command.");
				print_usage();
				lasm_common_exit(1);
			}

			source = option;
		}
	}

	if (NULL == entry)
	{
		entry = "main";
	}

	if (NULL == output)
	{
		output = source;
	}

	if (NULL == source)
	{
		lasm_logger_error("source file was not provided in 'build' command.");
		print_usage();
		lasm_common_exit(1);
	}

	lasm_arena_s arena = lasm_arena_new();
	lasm_lexer_s lexer = lasm_lexer_new(&arena, source);
	lasm_token_s token = lasm_token_new(lasm_token_type_none, lexer.location);
	while (!lasm_lexer_should_stop(lasm_lexer_lex(&lexer, &token)))
	{
		lasm_logger_log("%s", lasm_token_to_string(&token));
	}
	lasm_lexer_drop(&lexer);
	lasm_arena_drop(&arena);
}
