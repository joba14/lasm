
/**
 * @file common.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#ifndef __lasm__include__lasm__common_h__
#define __lasm__include__lasm__common_h__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

typedef bool bool_t;
typedef char char_t;

/**
 * @brief Wrapper for c's stdlib malloc function.
 * 
 * @note It will exit with code -1 if malloc fails to allocate heap region.
 * 
 * @param size size of to-be-allocated memory region
 * 
 * @return void*
 */
void* lasm_common_malloc(const uint64_t size) __attribute__((warn_unused_result));

/**
 * @brief Wrapper for c's stdlib realloc function.
 * 
 * @note It will exit with code -1 if malloc fails to reallocate heap region.
 * 
 * @param pointer pointer to the memory region to reallocate
 * @param size    size of to-be-allocated memory region
 * 
 * @return void*
 */
void* lasm_common_realloc(void* pointer, const uint64_t size) __attribute__((warn_unused_result));

/**
 * @brief Wrapper for c's stdlib free function.
 * 
 * @note After freeing the memory, the pointer will be set to NULL.
 * 
 * @param pointer pointer to the memory region to free
 */
void lasm_common_free(const void* pointer);

/**
 * @brief Wrapper for c's stdlib exit function.
 * 
 * @param code code to exit with
 */
void lasm_common_exit(const int32_t code);

/**
 * @brief Wrapper for c's stdlib memset function.
 * 
 * @param pointer pointer to a memory region to memset
 * @param value   value to set throughout the memory region 
 * @param length  length of the memory region
 */
void lasm_common_memset(void* const pointer, const uint8_t value, const uint64_t length);

/**
 * @brief Wrapper for c's stdlib memcpy function.
 * 
 * @param destination pointer to the destination memory region
 * @param source      pointer to the source memory region
 * @param length      length of the memory region to copy over
 */
void lasm_common_memcpy(void* const destination, const void* const source, const uint64_t length);

/**
 * @brief Wrapper for c's stdlib memcmp function.
 * 
 * @param left   left memory region
 * @param right  right memory region
 * @param length length of the memory regions
 * 
 * @return int32_t
 */
int32_t lasm_common_memcmp(const uint8_t* const left, const uint8_t* const right, const uint64_t length) __attribute__((warn_unused_result));

/**
 * @brief Wrapper for c's stdlib strlen function.
 * 
 * @param cstring pointer to a c string to have the length calculated of
 * 
 * @return uint64_t
 */
uint64_t lasm_common_strlen(const char_t* const cstring) __attribute__((warn_unused_result));

/**
 * @brief Wrapper for c's stdlib strcmp function.
 * 
 * @param left  left c string
 * @param right right c string
 * 
 * @return int32_t
 */
int32_t lasm_common_strcmp(const char_t* const left, const char_t* const right) __attribute__((warn_unused_result));

/**
 * @brief Wrapper for c's stdlib strcmp function.
 * 
 * @param left   left c string
 * @param right  right c string
 * @param length length of the strings
 * 
 * @return int32_t
 */
int32_t lasm_common_strncmp(const char_t* const left, const char_t* const right, const uint64_t length) __attribute__((warn_unused_result));

/**
 * @brief Wrapper for c's stdlib strchr function.
 * 
 * @param string string to search for symbol
 * @param c      symbol to search for
 * 
 * @return char_t*
 */
char_t* lasm_common_strchr(const char_t* const string, const int32_t c) __attribute__((warn_unused_result));

#endif
