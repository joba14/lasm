
/**
 * @file logger.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#ifndef __lasm__include__lasm__logger_h__
#define __lasm__include__lasm__logger_h__

#include "lasm/common.h"

#define lasm_red    "\033[91m"
#define lasm_yellow "\033[93m"
#define lasm_green  "\033[92m"
#define lasm_blue   "\033[34m"
#define lasm_reset  "\033[0m"

/**
 * @brief Log tagless level formattable messages.
 * 
 * @param format format of the log
 * @param ...    arguments of the log
 */
void lasm_logger_log(const char_t* const format, ...) __attribute__ ((format (printf, 1, 2)));

#ifndef NDEBUG
/**
 * @brief Log debug level formattable messages.
 * 
 * @param format format of the log
 * @param ...    arguments of the log
 */
void _lasm_logger_debug_impl(const char_t* const format, ...) __attribute__ ((format (printf, 1, 2)));

#	define lasm_logger_debug(_format, ...)                                     \
		_lasm_logger_debug_impl(_format, ##__VA_ARGS__)
#else
#	define lasm_logger_debug(_format, ...)                                     \
		do                                                                     \
		{                                                                      \
			(void)(_format);                                                   \
			(void)(__VA_ARGS__);                                               \
		} while (0)
#endif

/**
 * @brief Log info level formattable messages.
 * 
 * @param format format of the log
 * @param ...    arguments of the log
 */
void lasm_logger_info(const char_t* const format, ...) __attribute__ ((format (printf, 1, 2)));

/**
 * @brief Log warn level formattable messages.
 * 
 * @param format format of the log
 * @param ...    arguments of the log
 */
void lasm_logger_warn(const char_t* const format, ...) __attribute__ ((format (printf, 1, 2)));

/**
 * @brief Log error level formattable messages.
 * 
 * @param format format of the log
 * @param ...    arguments of the log
 */
void lasm_logger_error(const char_t* const format, ...) __attribute__ ((format (printf, 1, 2)));

#endif
