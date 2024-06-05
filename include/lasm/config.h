
/**
 * @file config.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-05
 */

#ifndef __lasm__include__lasm__config_h__
#define __lasm__include__lasm__config_h__

#include "lasm/common.h"
#include "lasm/arena.h"

typedef enum
{
	lasm_arch_type_x64_86,
	lasm_arch_type_arn16,
	lasm_arch_types_count,
	lasm_arch_type_none,
} lasm_arch_type_e;

lasm_arch_type_e lasm_arch_type_from_string(const char_t* const arch_as_string);

typedef enum
{
	lasm_format_type_elf,
	lasm_format_type_pe32plus,
	lasm_format_types_count,
	lasm_format_type_none,
} lasm_format_type_e;

lasm_format_type_e lasm_format_type_from_string(const char_t* const format_as_string);

typedef struct
{
	const char_t* arch;
	const char_t* format;
	const char_t* entry;
	const char_t* output;
	const char_t* source;
} lasm_config_s;

lasm_config_s lasm_config_from_cli(lasm_arena_s* const arena, int32_t* const argc, const char_t*** const argv);

#endif
