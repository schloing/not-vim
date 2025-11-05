#include "cursor.h"
#include "cursorhelp.h"
#include "buffer.h"
#include "editor.h"
#include "termbox2.h"

void nv_cursor_insert_ch(struct nv_context* ctx, struct cursor* cursor, char ch)
{
//  struct nv_node* l = line(ctx, cursor->line);
//  size_t row = l->begin + (cursor->x > l->length ? l->length : cursor->x);

//  cvector_insert(ctx->view->buffer->buffer, row, ch);

    cursor->x++;
}

// FIXME

void nv_cursor_move_down(struct nv_context* ctx, struct cursor* cursor, int amt)
{
    if (amt < 0) {
        nv_cursor_move_up(ctx, cursor, -amt);
        return;
    }

    if (cursor->y + amt < (int)ctx->buffer->line_count) {
        cursor->y += amt;
        cursor->line += amt;
    }
}

void nv_cursor_move_up(struct nv_context* ctx, struct cursor* cursor, int amt)
{
    if (amt < 0) {
        nv_cursor_move_down(ctx, cursor, -amt);
        return;
    }

    if (cursor->y - amt >= 0) {
        cursor->y -= amt;
        cursor->line -= amt;
    }
}

void nv_cursor_move_x(struct nv_context* ctx, struct cursor* cursor, int amt)
{
    struct nv_tree_node* l = NODE_FROM_POOL(line(ctx, cursor->line));
    int length = l ? (int)l->data.length : 0;

    // cursor could be out of range before any change to the cursor
    if (cursor->x > length)
        cursor->x = length;

    cursor->x += amt;

    if (cursor->x > length)
        cursor->x = length;

    if (cursor->x < 0)
        cursor->x = 0;
}

// struct cursor* nv_primary_cursor(struct nv_buff* buffer)
// {
//     return &buffer->cursors[NV_PRIMARY_CURSOR];
// }
