
/**
 * @file logger.c
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

#include <stdarg.h>
#include <stdio.h>

static void _log_with_tag(FILE* const stream, const char_t* const tag, const char_t* const format, va_list args);

void lasm_logger_log(const char_t* const format, ...)
{
	lasm_debug_assert(format != NULL);
	va_list args; va_start(args, format);
	_log_with_tag(stdout, NULL, format, args);
	va_end(args);
}

#ifndef NDEBUG
void _lasm_logger_debug_impl(const char_t* const format, ...)
{
	lasm_debug_assert(format != NULL);
	va_list args; va_start(args, format);
	_log_with_tag(stdout, lasm_blue "debug" lasm_reset, format, args);
	va_end(args);
}
#endif

void lasm_logger_info(const char_t* const format, ...)
{
	lasm_debug_assert(format != NULL);
	va_list args; va_start(args, format);
	_log_with_tag(stdout, lasm_green "info" lasm_reset, format, args);
	va_end(args);
}

void lasm_logger_warn(const char_t* const format, ...)
{
	lasm_debug_assert(format != NULL);
	va_list args; va_start(args, format);
	_log_with_tag(stderr, lasm_yellow "warn" lasm_reset, format, args);
	va_end(args);
}

void lasm_logger_error(const char_t* const format, ...)
{
	lasm_debug_assert(format != NULL);
	va_list args; va_start(args, format);
	_log_with_tag(stderr, lasm_red "error" lasm_reset, format, args);
	va_end(args);
}

static void _log_with_tag(FILE* const stream, const char_t* const tag, const char_t* const format, va_list args)
{
	lasm_debug_assert(stream != NULL);
	lasm_debug_assert(format != NULL);

	if (tag != NULL)
	{
		(void)fprintf(stream, "%s: ", tag);
	}

	(void)vfprintf(stream, format, args);
	(void)fprintf(stream, "\n");
}
