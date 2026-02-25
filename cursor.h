#ifndef NV_CURSOR_H
#define NV_CURSOR_H

#include <stddef.h>
#include "context.h"

struct cursor {
    int x;
    int y;
    int line;
    char ch;
    size_t buff_id;
};

#define NV_CURSOR_CAP 5
#define NV_PRIMARY_CURSOR 0

void nv_cursor_insert_ch(struct nv_context* ctx, struct cursor* cursor, char ch);
void nv_cursor_move_up(struct nv_context* ctx, struct cursor* cursor, int amt);
void nv_cursor_move_down(struct nv_context* ctx, struct cursor* cursor, int amt);
void nv_cursor_move_x(struct nv_context* ctx, struct cursor* cursor, int amt);

#endif
