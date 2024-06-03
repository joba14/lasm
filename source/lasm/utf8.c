
/**
 * @file utf8.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#include "lasm/debug.h"
#include "lasm/utf8.h"

static const uint8_t g_masks[] =
{
	0x7F,
	0x1F,
	0x0F,
	0x07,
	0x03,
	0x01
};

static const struct
{
	uint8_t mask;
	uint8_t result;
	int8_t octets;
} g_sizes[] =
{
	{ 0x80, 0x00,  1 },
	{ 0xE0, 0xC0,  2 },
	{ 0xF0, 0xE0,  3 },
	{ 0xF8, 0xF0,  4 },
	{ 0xFC, 0xF8,  5 },
	{ 0xFE, 0xFC,  6 },
	{ 0x80, 0x80, -1 },
};

/**
 * @brief Get the utf-8 size.
 * 
 * @param c char to get the size from
 * 
 * @return int8_t
 */
static int8_t get_utf8_size(const uint8_t c);

uint32_t lasm_utf8_decode(const char_t** const string)
{
	lasm_debug_assert(string != NULL);
	const uint8_t** s = (const uint8_t**)string;
	uint32_t cp = 0;

	if (**s < 128)
	{
		cp = **s;
		++(*s);
		return cp;
	}

	int8_t size = get_utf8_size(**s);

	if (-1 == size)
	{
		++(*s);
		return lasm_utf8_invalid;
	}

	const uint8_t mask = g_masks[size - 1];
	cp = **s & mask; ++(*s);

	while (--size)
	{
		const uint8_t c = **s; ++*s;

		if ((c >> 6) != 0x02)
		{
			return lasm_utf8_invalid;
		}

		cp <<= 6; cp |= c & 0x3f;
	}

	return cp;
}

uint8_t lasm_utf8_encode(char_t* const string, utf8char_t c)
{
	lasm_debug_assert(string != NULL);
	lasm_debug_assert(c != lasm_utf8_invalid);
	uint8_t length_minus_one, first;

	if (c < 0x80)
	{
		first = 0;
		length_minus_one = 0;
	}
	else if (c < 0x800)
	{
		first = 0xc0;
		length_minus_one = 1;
	}
	else if (c < 0x10000)
	{
		first = 0xe0;
		length_minus_one = 2;
	}
	else
	{
		first = 0xf0;
		length_minus_one = 3;
	}

	for (uint8_t index = length_minus_one; index > 0; --index)
	{
		string[index] = (char_t)((c & 0x3f) | 0x80);
		c >>= 6;
	}

	++length_minus_one;
	string[0] = (char_t)(c | first);
	return length_minus_one;
}

utf8char_t lasm_utf8_get(FILE* const file)
{
	lasm_debug_assert(file != NULL);
	char_t buffer[lasm_utf8_max_size];
	const int32_t c = fgetc(file);

	if (EOF == c)
	{
		return lasm_utf8_invalid;
	}

	buffer[0] = (char_t)c;
	const int8_t size = get_utf8_size((uint8_t)c);

	if (size > (int8_t)lasm_utf8_max_size)
	{
		(void)fseek(file, size - 1, SEEK_CUR);
		return lasm_utf8_invalid;
	}

	if (size > 1)
	{
		const size_t amt = fread(
			&buffer[1], 1, (size_t)(size - 1), file
		);

		if (amt != (size_t)(size - 1))
		{
			return lasm_utf8_invalid;
		}
	}

	const char_t* ptr = buffer;
	return lasm_utf8_decode(&ptr);
}

static int8_t get_utf8_size(const uint8_t c)
{
	const uint8_t count = (uint8_t)(
		sizeof(g_sizes) / sizeof(g_sizes[0])
	);

	for (uint8_t i = 0; i < count; ++i)
	{
		if ((c & g_sizes[i].mask) == g_sizes[i].result)
		{
			return g_sizes[i].octets;
		}
	}

	return -1;
}
