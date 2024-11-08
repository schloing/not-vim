#ifndef GENERAL_H
#define GENERAL_H

#include <stdint.h>

typedef struct {
    void* data;
    size_t peek;
    size_t index;
    size_t size;
} Vector;

Vector* new_vector(size_t size);
void*   push_vector(Vector* dest, void* elem);
void    free_vector(Vector* vec);

#endif
