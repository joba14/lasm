
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
#include "lasm/parser.h"

static const char_t* g_program = NULL;

static const char_t* const g_usage_banner =
	"usage: %s <command>\n"
	"\n"
	"commands:\n"
	"    build [options] <source.lasm>       build the project with provided source file.\n"
	"        required:\n"
	"            -a, --arch <name>           set the target architecture for the executable. supported architectures\n"
	"                                        are: %s.\n"
	"            -f, --format <name>         set the target format for the executable. supported formats are:\n"
	"                                        %s.\n"
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

typedef enum
{
	lasm_arch_type_x64_86,
	lasm_arch_type_arn16,
	lasm_arch_types_count,
	lasm_arch_type_none,
} lasm_arch_type_e;

typedef enum
{
	lasm_format_type_elf,
	lasm_format_type_pe32plus,
	lasm_format_types_count,
	lasm_format_type_none,
} lasm_format_type_e;

static const char_t* _g_supported_archs[lasm_arch_types_count] =
{
	[lasm_arch_type_x64_86] = "x64_86",
	[lasm_arch_type_arn16]  = "arn16" ,
};

static const char_t* _g_supported_formats[lasm_format_types_count] =
{
	[lasm_format_type_elf]      = "elf"  ,
	[lasm_format_type_pe32plus] = "pe32+",
};

static lasm_arch_type_e _lasm_arch_type_from_string(const char_t* const arch_as_string);

static lasm_format_type_e _lasm_format_type_from_string(const char_t* const format_as_string);

const char_t* _get_file_name_from_path(const char_t* const path);

static const char_t* _supported_archs_to_string(void);

static const char_t* _supported_formats_to_string(void);

static void _print_usage_banner(void);

static const char_t* _shift_cli_args(int32_t* const argc, const char_t*** const argv);

static bool_t _match_cli_option(const char_t* const option, const char_t* const long_name, const char_t* const short_name);

static const char_t* _get_option_argument(const char_t* const option, int32_t* const argc, const char_t*** const argv);

static void _run_build_command(int32_t* const argc, const char_t*** const argv);

int32_t main(int32_t argc, const char_t** argv)
{
	lasm_debug_assert(argc > 0);
	lasm_debug_assert(argv != NULL);

	lasm_debug_assert(NULL == g_program);
	g_program = _shift_cli_args(&argc, &argv);
	lasm_debug_assert(g_program != NULL);

	if (argc <= 0)
	{
		lasm_logger_error("no command was provided.");
		_print_usage_banner();
		lasm_common_exit(1);
	}

	const char_t* const command = _shift_cli_args(&argc, &argv);
	lasm_debug_assert(command != NULL);

	if (lasm_common_strcmp(command, "build") == 0)
	{
		_run_build_command(&argc, &argv);
	}
	else if (lasm_common_strcmp(command, "help") == 0)
	{
		_print_usage_banner();
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
		_print_usage_banner();
		lasm_common_exit(1);
	}

	return 0;
}

static lasm_arch_type_e _lasm_arch_type_from_string(const char_t* const arch_as_string)
{
	for (uint64_t index = 0; index < lasm_arch_types_count; ++index)
	{
		const char_t* arch = _g_supported_archs[index];
		lasm_debug_assert(arch != NULL);

		const uint64_t arch_length = lasm_common_strlen(arch);
		lasm_debug_assert(arch_length > 0);

		const uint64_t arch_as_string_length = lasm_common_strlen(arch_as_string);
		lasm_debug_assert(arch_as_string_length > 0);

		if ((arch_as_string_length == arch_length) && (lasm_common_strncmp(arch_as_string, arch, arch_as_string_length) == 0))
		{
			return index;
		}
	}

	return lasm_arch_type_none;
}

static lasm_format_type_e _lasm_format_type_from_string(const char_t* const format_as_string)
{
	for (uint64_t index = 0; index < lasm_format_types_count; ++index)
	{
		const char_t* format = _g_supported_formats[index];
		lasm_debug_assert(format != NULL);

		const uint64_t format_length = lasm_common_strlen(format);
		lasm_debug_assert(format_length > 0);

		const uint64_t format_as_string_length = lasm_common_strlen(format_as_string);
		lasm_debug_assert(format_as_string_length > 0);

		if ((format_as_string_length == format_length) && (lasm_common_strncmp(format_as_string, format, format_as_string_length) == 0))
		{
			return index;
		}
	}

	return lasm_format_type_none;
}

const char_t* _get_file_name_from_path(const char_t* const path)
{
	lasm_debug_assert(path != NULL);

	const uint64_t path_length = lasm_common_strlen(path);
	lasm_debug_assert(path_length > 0);

	for (uint64_t offset = path_length; offset > 0; --offset)
	{
		const uint64_t index = offset - 1;

		if (('\\' == path[index]) || ('/' == path[index]))
		{
			return path + offset;
		}
	}

	return path;
}

static const char_t* _supported_archs_to_string(void)
{
	#define archs_list_string_buffer_capacity 512
	static char_t archs_list_string_buffer[archs_list_string_buffer_capacity + 1];
	uint64_t written = 0;

	for (uint64_t index = 0; index < lasm_arch_types_count; ++index)
	{
		written += (uint64_t)snprintf(archs_list_string_buffer + written, archs_list_string_buffer_capacity - written, "%s", _g_supported_archs[index]);

		if (index < (lasm_arch_types_count - 1))
		{
			written += (uint64_t)snprintf(archs_list_string_buffer + written, archs_list_string_buffer_capacity - written, "%s", ", ");

			if (index == (lasm_arch_types_count - 2))
			{
				written += (uint64_t)snprintf(archs_list_string_buffer + written, archs_list_string_buffer_capacity - written, "%s", "and ");
			}
		}
	}

	return archs_list_string_buffer;
}

static const char_t* _supported_formats_to_string(void)
{
	#define formats_list_string_buffer_capacity 512
	static char_t formats_list_string_buffer[formats_list_string_buffer_capacity + 1];
	uint64_t written = 0;

	for (uint64_t index = 0; index < lasm_format_types_count; ++index)
	{
		written += (uint64_t)snprintf(formats_list_string_buffer + written, formats_list_string_buffer_capacity - written, "%s", _g_supported_formats[index]);

		if (index < (lasm_format_types_count - 1))
		{
			written += (uint64_t)snprintf(formats_list_string_buffer + written, formats_list_string_buffer_capacity - written, "%s", ", ");

			if (index == (lasm_format_types_count - 2))
			{
				written += (uint64_t)snprintf(formats_list_string_buffer + written, formats_list_string_buffer_capacity - written, "%s", "and ");
			}
		}
	}

	return formats_list_string_buffer;
}

static void _print_usage_banner(void)
{
	lasm_debug_assert(g_usage_banner != NULL);
	lasm_debug_assert(g_program != NULL);
	lasm_logger_log(g_usage_banner, g_program, _supported_archs_to_string(), _supported_formats_to_string());
}

static const char_t* _shift_cli_args(int32_t* const argc, const char_t*** const argv)
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

static bool_t _match_cli_option(const char_t* const option, const char_t* const long_name, const char_t* const short_name)
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

static const char_t* _get_option_argument(const char_t* const option, int32_t* const argc, const char_t*** const argv)
{
	lasm_debug_assert(option != NULL);
	lasm_debug_assert(argc != NULL);
	lasm_debug_assert(argv != NULL);

	const char_t* const argument = _shift_cli_args(argc, argv);

	if (NULL == argument)
	{
		lasm_logger_error("option '%s' requires an argument, but none was provided.", option);
		_print_usage_banner();
		lasm_common_exit(1);
	}

	return argument;
}

static void _run_build_command(int32_t* const argc, const char_t*** const argv)
{
	lasm_debug_assert(argc != NULL);
	lasm_debug_assert(argv != NULL);

	lasm_arena_s arena = lasm_arena_new();

	const char_t* arch   = NULL;
	const char_t* format = NULL;
	const char_t* entry  = NULL;
	const char_t* output = NULL;
	const char_t* source = NULL;

	for (uint64_t index = 0; true; ++index)
	{
		const char_t* const option = _shift_cli_args(argc, argv);

		if (NULL == option)
		{
			break;
		}

		if (_match_cli_option(option, "--arch", "-a"))
		{
			if (arch != NULL)
			{
				lasm_logger_error("multiple --arch, -a arguments found in the command line arguments in 'build' command.");
				_print_usage_banner();
				lasm_common_exit(1);
			}

			const char_t* const arch_as_string = _get_option_argument(option, argc, argv);
			lasm_debug_assert(arch_as_string != NULL);
			arch = arch_as_string;
		}
		else if (_match_cli_option(option, "--format", "-f"))
		{
			if (format != NULL)
			{
				lasm_logger_error("multiple --format, -f arguments found in the command line arguments in 'build' command.");
				_print_usage_banner();
				lasm_common_exit(1);
			}

			const char_t* const format_as_string = _get_option_argument(option, argc, argv);
			lasm_debug_assert(format_as_string != NULL);
			format = format_as_string;
		}
		else if (_match_cli_option(option, "--entry", "-e"))
		{
			if (entry != NULL)
			{
				lasm_logger_error("multiple --entry, -e arguments found in the command line arguments in 'build' command.");
				_print_usage_banner();
				lasm_common_exit(1);
			}

			const char_t* const entry_as_string = _get_option_argument(option, argc, argv);
			lasm_debug_assert(entry_as_string != NULL);
			entry = entry_as_string;
		}
		else if (_match_cli_option(option, "--output", "-o"))
		{
			if (output != NULL)
			{
				lasm_logger_error("multiple --output, -o arguments found in the command line arguments in 'build' command.");
				_print_usage_banner();
				lasm_common_exit(1);
			}

			const char_t* const output_as_string = _get_option_argument(option, argc, argv);
			lasm_debug_assert(output_as_string != NULL);
			output = output_as_string;
		}
		else
		{
			if (source != NULL)
			{
				lasm_logger_error("multiple source files found in the command line arguments in 'build' command.");
				_print_usage_banner();
				lasm_common_exit(1);
			}

			source = option;
		}
	}

	if (NULL == arch)
	{
		lasm_logger_error("no architecture was provided in the command line arguments in 'build' command. supported architectures are: %s.", _supported_archs_to_string());
		_print_usage_banner();
		lasm_common_exit(1);
	}
	else
	{
		if (lasm_arch_type_none == _lasm_arch_type_from_string(arch))
		{
			lasm_logger_error("an invalid architecture was provided in the command line arguments in 'build' command: %s. supported architectures are: %s.", arch, _supported_archs_to_string());
			_print_usage_banner();
			lasm_common_exit(1);
		}
	}

	if (NULL == format)
	{
		lasm_logger_error("no format was provided in the command line arguments in 'build' command. supported formats are: %s.", _supported_formats_to_string());
		_print_usage_banner();
		lasm_common_exit(1);
	}
	else
	{
		if (lasm_format_type_none == _lasm_format_type_from_string(format))
		{
			lasm_logger_error("an invalid format was provided in the command line arguments in 'build' command: %s. supported formats are: %s.", format, _supported_formats_to_string());
			_print_usage_banner();
			lasm_common_exit(1);
		}
	}

	if (NULL == entry)
	{
		entry = "main";
	}

	if (NULL == output)
	{
		const char_t* const source_name = _get_file_name_from_path(source);
		lasm_debug_assert(source_name != NULL);

		const uint64_t source_name_length = lasm_common_strlen(source_name);
		lasm_debug_assert(source_name_length > 0);

		const uint64_t format_length = lasm_common_strlen(format);
		lasm_debug_assert(format_length > 0);

		char_t* const output_path = (char_t* const)lasm_arena_alloc(&arena, source_name_length + 1 + format_length);
		lasm_debug_assert(output_path != NULL);

		lasm_common_memcpy(output_path, source_name, source_name_length);
		lasm_common_memcpy(output_path + source_name_length, ".", 1);
		lasm_common_memcpy(output_path + source_name_length + 1, format, format_length);
		output = output_path;
	}

	if (NULL == source)
	{
		lasm_logger_error("source file was not provided in 'build' command.");
		_print_usage_banner();
		lasm_common_exit(1);
	}

	{  // note: compilation and building starts here:
		lasm_parser_s parser = lasm_parser_new(&arena, source);

		lasm_ast_label_s* label = NULL;
		while ((label = lasm_parser_parse_label(&parser)) != NULL)
		{
			lasm_logger_debug("\n%s\n", lasm_ast_label_to_string(label));

			switch (_lasm_arch_type_from_string(arch))
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
		}

		lasm_parser_drop(&parser);
	}

	lasm_arena_drop(&arena);
}
