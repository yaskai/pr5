#pragma once

#include <stdlib.h>
#include "common/nums.h"

typedef struct {
	size_t capacity;
	size_t size;

	u8 *data;

} Arena;

Arena arena_Init(size_t capacity);
void *arena_Alloc(Arena *arena, size_t size);
void arena_Clear(Arena *arena);
void arena_Free(Arena *arena);

