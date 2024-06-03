
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

/**
 * @brief Log tagless level formattable messages.
 * 
 * @param format format of the log
 * @param ...    arguments of the log
 */
void lasm_logger_log(const char_t* const format, ...)
#ifndef _WIN32
	__attribute__ ((format (printf, 1, 2)));
#else
	;
#endif

/**
 * @brief Log info level formattable messages.
 * 
 * @param format format of the log
 * @param ...    arguments of the log
 */
void lasm_logger_info(const char_t* const format, ...)
#ifndef _WIN32
	__attribute__ ((format (printf, 1, 2)));
#else
	;
#endif

/**
 * @brief Log warn level formattable messages.
 * 
 * @param format format of the log
 * @param ...    arguments of the log
 */
void lasm_logger_warn(const char_t* const format, ...)
#ifndef _WIN32
	__attribute__ ((format (printf, 1, 2)));
#else
	;
#endif

/**
 * @brief Log error level formattable messages.
 * 
 * @param format format of the log
 * @param ...    arguments of the log
 */
void lasm_logger_error(const char_t* const format, ...)
#ifndef _WIN32
	__attribute__ ((format (printf, 1, 2)));
#else
	;
#endif

#endif
