#include "cursor.h"
#include "buffer.h"
#include "termbox2.h"
#include "window.h"

void nv_cursor_insert_ch(struct nv_context* ctx, struct cursor* cursor, char ch)
{
//  struct nv_node* l = line(ctx, cursor->line);
//  size_t row = l->begin + (cursor->x > l->length ? l->length : cursor->x);

//  cvector_insert(ctx->view->buffer->buffer, row, ch);

    cursor->x++;
}

void nv_cursor_move_down(struct nv_context* ctx, struct cursor* cursor, int amt)
{
    if (amt < 0) {
        nv_cursor_move_up(ctx, cursor, -amt);
        return;
    }

    if (cursor->line + amt <= ctx->buffer->line_count) {
        cursor->y += amt;
        cursor->line += amt;
    }

    if (cursor->y > ctx->window->leaf.area.h - 1) {
        ctx->view->top_line_index++;
        cursor->y = ctx->window->leaf.area.h - 1;
        cursor->line -= amt;
        cursor->line++;
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
    else {
        ctx->view->top_line_index--;
        cursor->y = 0;
        cursor->line--;
    }

    if (cursor->line <= 1) {
        cursor->line = 1;
    }

    if (ctx->view->top_line_index <= 1) {
        ctx->view->top_line_index = 1;
    }
}

void nv_cursor_move_x(struct nv_context* ctx, struct cursor* cursor, int amt)
{
    struct nv_node* l = nv_get_computed_line(ctx, cursor->line);

    if (!l) {
        return;
    }

    // cursor could be out of range before any change to the cursor
    if (cursor->x > l->length)
        cursor->x = l->length;

    cursor->x += amt;

    if (cursor->x > l->length)
        cursor->x = l->length;

    if (cursor->x < 0)
        cursor->x = 0;
}

// struct cursor* nv_primary_cursor(struct nv_buff* buffer)
// {
//     return &buffer->cursors[NV_PRIMARY_CURSOR];
// }
