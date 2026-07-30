#include "mem_arena.h"

Arena arena_Init(size_t capacity) {
	return (Arena) {
		.capacity = capacity,
		.size = 0,
		.data = (u8*)malloc(capacity)
	};
}

void *arena_Alloc(Arena *arena, size_t size) {
	void *data = &arena->data[arena->size];
	arena->size += size;

	return data;
}

void arena_Clear(Arena *arena) {
	arena->size = 0;
}

void arena_Free(Arena *arena) {
	free(arena->data);
	*arena = (Arena) {0};
}

