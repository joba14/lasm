
/**
 * @file arena.c
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#include "lasm/debug.h"
#include "lasm/logger.h"
#include "lasm/arena.h"

lasm_arena_node_s* lasm_arena_node_new(const uint64_t size)
{
	lasm_debug_assert(size > 0);

	lasm_arena_node_s* const node = (lasm_arena_node_s* const)lasm_common_malloc(sizeof(lasm_arena_node_s));
	lasm_debug_assert(node != NULL);

	void* const pointer = (void* const)lasm_common_malloc(size);
	lasm_debug_assert(pointer != NULL);

	*node = (lasm_arena_node_s)
	{
		.pointer = pointer,
		.size = size,
		.next = NULL,
	};

	return node;
}

void lasm_arena_node_drop(lasm_arena_node_s* node)
{
	lasm_debug_assert(node != NULL);
	lasm_debug_assert(node->pointer != NULL);
	node->pointer = lasm_common_free(node->pointer);
	(void)lasm_common_free(node);
	*node = (lasm_arena_node_s) {0};
}

lasm_arena_s lasm_arena_new(void)
{
	return (lasm_arena_s) {0};
}

void lasm_arena_drop(lasm_arena_s* const arena)
{
	lasm_debug_assert(arena != NULL);
	const lasm_arena_node_s* node_iterator = arena->begin;

	while (node_iterator != NULL)
	{
		const lasm_arena_node_s* const node = node_iterator;
		lasm_debug_assert(node != NULL);
		node_iterator = node_iterator->next;
		lasm_arena_node_drop((lasm_arena_node_s* const)node);
	}

	arena->begin = NULL;
	arena->end = NULL;
}

void* lasm_arena_alloc(lasm_arena_s* const arena, const uint64_t size)
{
	lasm_debug_assert(arena != NULL);
	lasm_debug_assert(size > 0);

	if (NULL == arena->end)
	{
		lasm_debug_assert(NULL == arena->begin);
		arena->end = lasm_arena_node_new(size);
		lasm_debug_assert(arena->end != NULL);
		arena->begin = arena->end;
	}
	else
	{
		while (arena->end->next != NULL)
		{
			arena->end = arena->end->next;
		}

		arena->end->next = lasm_arena_node_new(size);
		lasm_debug_assert(arena->end->next != NULL);
		arena->end = arena->end->next;
	}

	void* const result = arena->end->pointer;
	lasm_debug_assert(result != NULL);
	return result;
}
