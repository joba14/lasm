
/**
 * @file ast.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-06
 */

#include "lasm/ast.h"
#include "lasm/debug.h"
#include "lasm/logger.h"

#include <stdio.h>

lasm_implement_vector_type(lasm_bytes_vector, uint8_t);

const char_t* lasm_ast_perm_type_to_string(const lasm_ast_perm_type_e type)
{
	switch (type)
	{
		case lasm_ast_perm_type_r:   { return "r";   } break;
		case lasm_ast_perm_type_rw:  { return "rw";  } break;
		case lasm_ast_perm_type_rx:  { return "rx";  } break;
		case lasm_ast_perm_type_rwx: { return "rwx"; } break;

		default:
		{
			lasm_debug_assert(0);  // note: sanity check for developers.
			return NULL;
		} break;
	}
}

const char_t* lasm_ast_label_to_string(const lasm_ast_label_s* const label)
{
	lasm_debug_assert(label != NULL);

	#define label_string_buffer_capacity 4096
	static char_t label_string_buffer[label_string_buffer_capacity + 1];
	uint64_t written = 0;

	const lasm_ast_attr_s addr_attr = label->attrs[lasm_ast_attr_type_addr];
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "[addr=");
	if (addr_attr.inferred) { written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "auto"); }
	else { written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%lu", addr_attr.as.addr.value); }
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", ", ");

	const lasm_ast_attr_s align_attr = label->attrs[lasm_ast_attr_type_align];
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "align=");
	if (align_attr.inferred) { written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "auto"); }
	else { written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%lu", align_attr.as.align.value); }
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", ", ");

	const lasm_ast_attr_s size_attr = label->attrs[lasm_ast_attr_type_size];
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "size=");
	if (size_attr.inferred) { written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "auto"); }
	else { written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%lu", size_attr.as.size.value); }
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", ", ");

	const lasm_ast_attr_s perm_attr = label->attrs[lasm_ast_attr_type_perm];
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "perm=");
	if (perm_attr.inferred) { written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "auto"); }
	else { written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", lasm_ast_perm_type_to_string(perm_attr.as.perm.value)); }
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", ",]\n");

	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s:\n", label->name);
	for (uint64_t index = 0; index < label->body.count; ++index) { written += (uint64_t)snprintf(label_string_buffer + written,
	label_string_buffer_capacity - written, "    0x%02X\n", *lasm_bytes_vector_at((lasm_bytes_vector_s* const)&label->body, index)); }
	written += (uint64_t)snprintf(label_string_buffer + written, label_string_buffer_capacity - written, "%s", "end");

	return label_string_buffer;
}

lasm_implement_vector_type(lasm_labels_vector, lasm_ast_label_s);
