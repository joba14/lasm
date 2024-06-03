
/**
 * @file vector.h
 * 
 * @copyright This file is a part of the "lasm" project and is distributed, and
 * licensed under "lasm gplv1" license.
 * 
 * @author joba14
 * 
 * @date 2024-06-03
 */

#ifndef __lasm__include__lasm__vector_h__
#define __lasm__include__lasm__vector_h__

#include "lasm/common.h"
#include "lasm/debug.h"
#include "lasm/arena.h"

#define raven_define_vector_type(_type_name, _element_type)                    \
	typedef struct                                                             \
	{                                                                          \
		raven_arena_s* arena;                                                  \
		_element_type* data;                                                   \
		uint64_t capacity;                                                     \
		uint64_t count;                                                        \
	} _type_name ## _s;                                                        \
	                                                                           \
	_type_name ## _s _type_name ## _new(raven_arena_s* const arena,            \
		                                const uint64_t capacity);              \
	                                                                           \
	void _type_name ## _push(_type_name ## _s* const vector,                   \
		                      _element_type element);                          \
	                                                                           \
	bool_t _type_name ## _pop(_type_name ## _s* const vector,                  \
		                      _element_type* const element);                   \
	                                                                           \
	_element_type* _type_name ## _at(_type_name ## _s* const vector,           \
		                             const uint64_t index);                    \
	                                                                           \
	_Static_assert(1, "")  // note: left for ';' support after calling the macro.

#define raven_implement_vector_type(_type_name, _element_type)                 \
	_type_name ## _s _type_name ## _new(raven_arena_s* const arena,            \
		                                const uint64_t capacity)               \
	{                                                                          \
		raven_debug_assert(arena != NULL);                                     \
		raven_debug_assert(capacity > 0);                                      \
		                                                                       \
		_element_type* const data = (_element_type* const)raven_arena_alloc(   \
			arena, capacity * sizeof(_element_type));                          \
		raven_debug_assert(data != NULL);                                      \
		                                                                       \
		return (_type_name ## _s)                                              \
		{                                                                      \
			.arena    = arena,                                                 \
			.data     = data,                                                  \
			.capacity = capacity,                                              \
			.count    = 0,                                                     \
		};                                                                     \
	}                                                                          \
	                                                                           \
	void _type_name ## _push(_type_name ## _s* const vector,                   \
		                     _element_type element)                            \
	{                                                                          \
		raven_debug_assert(vector != NULL);                                    \
		                                                                       \
		if (vector->count + 1 >= vector->capacity)                             \
		{                                                                      \
			const uint64_t new_capacity = (uint64_t)(                          \
				vector->capacity + (vector->capacity / 2) + 1                  \
			);                                                                 \
			                                                                   \
			_element_type* new_data = raven_arena_alloc(                       \
				vector->arena, new_capacity * sizeof(_element_type));          \
			raven_debug_assert(new_data != NULL);                              \
			                                                                   \
			if (vector->count > 0)                                             \
			{                                                                  \
				raven_memcpy(new_data, vector->data,                           \
					vector->count * sizeof(_element_type)                      \
				);                                                             \
			}                                                                  \
			                                                                   \
			vector->data = new_data;                                           \
			vector->capacity = new_capacity;                                   \
		}                                                                      \
		                                                                       \
		vector->data[vector->count++] = element;                               \
	}                                                                          \
	                                                                           \
	bool_t _type_name ## _pop(_type_name ## _s* const vector,                  \
		                      _element_type* const element)                    \
	{                                                                          \
		raven_debug_assert(vector != NULL);                                    \
		raven_debug_assert(element != NULL);                                   \
		                                                                       \
		if (vector->count <= 0)                                                \
		{                                                                      \
			return false;                                                      \
		}                                                                      \
		                                                                       \
		*element = vector->data[--vector->count];                              \
		return true;                                                           \
	}                                                                          \
	                                                                           \
	_element_type* _type_name ## _at(_type_name ## _s* const vector,           \
		                             const uint64_t index)                     \
	{                                                                          \
		raven_debug_assert(vector != NULL);                                    \
		raven_debug_assert(index < vector->count);                             \
		return &vector->data[index];                                           \
	}                                                                          \
	                                                                           \
	_Static_assert(1, "")  // note: left for ';' support after calling the macro.

#endif
