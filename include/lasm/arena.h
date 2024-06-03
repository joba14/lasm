
/**
 * @file arena.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#ifndef __lasm__include__lasm__arena_h__
#define __lasm__include__lasm__arena_h__

#include "lasm/common.h"

typedef struct lasm_arena_node_s lasm_arena_node_s;

struct lasm_arena_node_s
{
	void* pointer;
	uint64_t size;
	lasm_arena_node_s* next;
};

/**
 * @brief Create arena node, allocate it's pointer with provided size.
 * 
 * @param size size of to-be-allocated memory block
 * 
 * @return lasm_arena_node_s*
 */
lasm_arena_node_s* lasm_arena_node_new(const uint64_t size);

/**
 * @brief Destroy and deallocate the inner pointer and the arena node itself.
 * 
 * @param node arena node to destroy
 */
void lasm_arena_node_drop(lasm_arena_node_s* node);

typedef struct
{
	lasm_arena_node_s* begin;
	lasm_arena_node_s* end;
} lasm_arena_s;

/**
 * @brief Create arena object.
 * 
 * @return lasm_arena_s
 */
lasm_arena_s lasm_arena_new(void);

/**
 * @brief Destroy and deallocate the entire arena (and it's nodes).
 * 
 * @param arena arena instance
 */
void lasm_arena_drop(lasm_arena_s* const arena);

/**
 * @brief Allocate a region of memory with provided size and store the pointer
 * to it wihtin arena's node.
 * 
 * @param arena arena instance
 * @param size  size of to-be-allocated memory block
 * 
 * @return void*
 */
void* lasm_arena_alloc(lasm_arena_s* const arena, const uint64_t size);

#endif
