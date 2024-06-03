
/**
 * @file common.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#include "lasm/common.h"
#include "lasm/debug.h"
#include "lasm/logger.h"

#include <stdlib.h>
#include <memory.h>
#include <string.h>

void* lasm_common_malloc(const uint64_t size)
{
	lasm_debug_assert(size > 0);
	void* const pointer = (void* const)malloc(size);

	if (NULL == pointer)
	{
		lasm_logger_error("internal failure -- failed to allocate memory.");
		lasm_common_exit(1);
	}

	return pointer;
}

void* lasm_common_realloc(void* pointer, const uint64_t size)
{
	lasm_debug_assert(size > 0);
	pointer = (void*)realloc(pointer, size);

	if (NULL == pointer)
	{
		lasm_logger_error("internal failure -- failed to reallocate memory.");
		lasm_common_exit(1);
	}

	return pointer;
}

void* lasm_common_free(const void* pointer)
{
	free((void*)pointer);
	return NULL;
}

void lasm_common_exit(const int32_t code)
{
	exit(code);
}

void lasm_common_memset(void* const pointer, const uint8_t value, const uint64_t length)
{
	lasm_debug_assert(pointer != NULL);
	(void)memset((void*)pointer, value, length);
}

void lasm_common_memcpy(void* const destination, const void* const source, const uint64_t length)
{
	lasm_debug_assert(destination != NULL);
	lasm_debug_assert(source != NULL);
	lasm_debug_assert(length > 0);
	(void)memcpy((void*)destination, (const void*)source, length);
}

int32_t lasm_common_memcmp(const uint8_t* const left, const uint8_t* const right, const uint64_t length)
{
	return (int32_t)(memcmp(left, right, length));
}

uint64_t lasm_common_strlen(const char_t* const cstring)
{
	lasm_debug_assert(cstring != NULL);
	return (uint64_t)strlen(cstring);
}

int32_t lasm_common_strcmp(const char_t* const left, const char_t* const right)
{
	lasm_debug_assert(left != NULL);
	lasm_debug_assert(right != NULL);
	return strcmp((const char_t*)left, (const char_t*)right);
}

int32_t lasm_common_strncmp(const char_t* const left, const char_t* const right, const uint64_t length)
{
	lasm_debug_assert(left != NULL);
	lasm_debug_assert(right != NULL);
	return strncmp((const char_t*)left, (const char_t*)right, (size_t)length);
}

char_t* lasm_common_strchr(const char_t* const string, const int32_t c)
{
	lasm_debug_assert(string != NULL);
	return strchr(string, c);
}
