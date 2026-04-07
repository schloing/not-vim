#ifndef NV_ARENA_H
#define NV_ARENA_H

#include <stddef.h>

struct nv_arena_alloc_head;

enum nv_arena_block_status {
    NV_ARENA_BLOCK_STATUS_RESERVED,
    NV_ARENA_BLOCK_STATUS_FREE,
};

struct nv_arena_alloc_head {
    size_t size;
    enum nv_arena_block_status status;
    struct nv_arena_alloc_head* next;
};

void nv_arena_init();
void nv_arena_free_all();
void nv_arena_free(void* ptr);
void* nv_arena_malloc(size_t size);
void* nv_arena_calloc(size_t nmemb, size_t size);
void* nv_arena_realloc(void* ptr, size_t size);

#define NV_ARENA_DEFAULT_ALLOC_SIZE 1024 * 1024 * 10

#endif