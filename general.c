#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "general.h"

Vector* new_vector(size_t size) {
    Vector* vec = (Vector*)calloc(1, sizeof(Vector));

    vec->size = size;
    vec->data = malloc(size);

    assert(vec->data);

    if (!vec->data) {
        fprintf(stderr, "allocation failed\n");
        exit(EXIT_FAILURE);
    }

    vec->peek = vec->index = 0;

    return vec;
}

void* push_vector(Vector* dest, void* elem) {
    assert(dest->data);

    if (dest->index++ >= dest->size) {
        dest->size *= 2;
        dest->data = (void*)realloc(dest->data, dest->size);

        assert(dest->data);
    }

    void* dest_ptr = dest->data + dest->index;
    memcpy(dest_ptr, elem, sizeof(void*));

    return dest_ptr;
}

void free_vector(Vector* vec) {
    free(vec->data);

    vec->data = NULL;
    vec->size = -1;
}
