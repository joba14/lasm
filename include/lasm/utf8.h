
/**
 * @file utf8.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#ifndef __lasm__include__lasm__utf8_h__
#define __lasm__include__lasm__utf8_h__

#include "lasm/common.h"

#include <stdio.h>

typedef uint32_t utf8char_t;
#define lasm_utf8_max_size sizeof(utf8char_t)
#define lasm_utf8_invalid UINT32_MAX

/**
 * @brief Decode utf-8 char from provided string.
 * 
 * @param string string to decode from
 * 
 * @return utf8char_t
 */
utf8char_t lasm_utf8_decode(const char_t** const string);

/**
 * @brief Encode utf-8 char into a provided string.
 * 
 * @param string string to encode into
 * @param c      utf-8 char to encode
 * 
 * @return uint8_t
 */
uint8_t lasm_utf8_encode(char_t* const string, utf8char_t c);

/**
 * @brief Get utf-8 char from file.
 * 
 * @param file file to read from
 * 
 * @return utf8char_t
 */
utf8char_t lasm_utf8_get(FILE* const file);

#endif
