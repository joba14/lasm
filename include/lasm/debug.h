
/**
 * @file debug.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#ifndef __lasm__include__lasm__debug_h__
#define __lasm__include__lasm__debug_h__

#ifndef NDEBUG
#	include "lasm/common.h"

/**
 * @brief Debug assert implementation.
 * 
 * @param expression        expression to evaluate
 * @param expression_string stringified expression
 * @param file              file in which assert exists
 * @param line              line in which assert exists
 */
void _lasm_debug_assert_impl(const bool_t expression, const char_t* const expression_string, const char_t* const file, const uint64_t line);

/**
 * @brief Debug assert wrapper.
 * 
 * @note It abstracts the @ref _lasm_debug_assert_impl function and passes line,
 * file, and stringified expression to the implementation function.
 */
#	define lasm_debug_assert(_expression)                                      \
		_lasm_debug_assert_impl(                                               \
			_expression,                                                       \
			#_expression,                                                      \
			(const char_t* const)__FILE__,                                     \
			(const uint64_t)__LINE__                                           \
		)
#else
/**
 * @brief Debug assert wrapper.
 * 
 * @note It abstracts the @ref _lasm_debug_assert_impl function and passes line,
 * file, and stringified expression to the implementation function.
 */
#	define lasm_debug_assert(_expression) ((void)(_expression))
#endif

#endif
