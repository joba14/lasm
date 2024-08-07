
/**
 * @file ast.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-06
 */

#ifndef __lasm__include__lasm__ast_h__
#define __lasm__include__lasm__ast_h__

#include "lasm/common.h"
#include "lasm/vector.h"
#include "lasm/token.h"

lasm_define_vector_type(lasm_bytes_vector, uint8_t);

typedef enum
{
	lasm_ast_attr_type_addr,
	lasm_ast_attr_type_align,
	lasm_ast_attr_type_size,
	lasm_ast_attr_type_perm,
	lasm_ast_attr_types_count,
} lasm_ast_attr_type_e;

typedef struct
{
	uint64_t value;
} lasm_ast_attr_addr_s;

typedef struct
{
	uint64_t value;
} lasm_ast_attr_align_s;

typedef struct
{
	uint64_t value;
} lasm_ast_attr_size_s;

typedef enum
{
	lasm_ast_perm_type_r,
	lasm_ast_perm_type_rw,
	lasm_ast_perm_type_rx,
	lasm_ast_perm_type_rwx,
	lasm_ast_perm_type_none,
} lasm_ast_perm_type_e;

const char_t* lasm_ast_perm_type_to_string(const lasm_ast_perm_type_e type);

typedef struct
{
	lasm_ast_perm_type_e value;
} lasm_ast_attr_perm_s;

typedef struct
{
	lasm_ast_attr_type_e type;
	bool_t inferred;

	union
	{
		lasm_ast_attr_addr_s addr;
		lasm_ast_attr_align_s align;
		lasm_ast_attr_size_s size;
		lasm_ast_attr_perm_s perm;
	} as;
} lasm_ast_attr_s;

typedef struct
{
	lasm_location_s location;
	lasm_ast_attr_s attrs[lasm_ast_attr_types_count];
	const char_t* name;
	lasm_tokens_vector_s body_tokens;
	lasm_bytes_vector_s body;
} lasm_ast_label_s;

const char_t* lasm_ast_label_to_string(const lasm_ast_label_s* const label);

lasm_define_vector_type(lasm_labels_vector, lasm_ast_label_s);

#endif
