#ifndef NV_CURSOR_H
#define NV_CURSOR_H

#include <stddef.h>
#include "context.h"

struct cursor {
    size_t col;
    size_t line; // 1-indexed
    size_t offset; // absolute buffer index
};

#define NV_CURSOR_CAP 5
#define NV_PRIMARY_CURSOR 0

void nv_cursor_insert_ch(struct nv_context* ctx, struct cursor* cursor, char ch);
void nv_cursor_move_up(struct nv_context* ctx, struct cursor* cursor, int amt);
void nv_cursor_move_down(struct nv_context* ctx, struct cursor* cursor, int amt);
void nv_cursor_move_x(struct nv_context* ctx, struct cursor* cursor, int amt);
struct cursor* nv_primary_cursor(struct nv_context* ctx);

#endif
