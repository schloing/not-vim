#include "cursor.h"
#include "buffer.h"
#include "context.h"
#include "editor.h"
#include "termbox2.h"
#include "window.h"

void nv_cursor_insert_ch(struct nv_context* ctx, struct cursor* cursor, char ch)
{
    // TODO: implement
    nv_log("unimplemented %s\n", __PRETTY_FUNCTION__);
    (void)ctx;
    (void)cursor;
    (void)ch;
    cursor->x++;
}

void nv_cursor_move_down(struct nv_context* ctx, struct cursor* cursor, int amt)
{
    if (amt < 0) {
        nv_cursor_move_up(ctx, cursor, -amt);
        return;
    }

    cursor->line += amt;

    if (cursor->line > ctx->buffer->line_count) {
        cursor->line = ctx->buffer->line_count;
    }

    if (cursor->line >= ctx->view->top_line_index + ctx->window->leaf.area.h) {
        ctx->view->top_line_index++;
    }

    if (ctx->view->top_line_index > ctx->buffer->line_count) {
        ctx->view->top_line_index = ctx->buffer->line_count;
    }
}

void nv_cursor_move_up(struct nv_context* ctx, struct cursor* cursor, int amt)
{
    if (amt < 0) {
        nv_cursor_move_down(ctx, cursor, -amt);
        return;
    }

    cursor->line -= amt;

    if (cursor->line < 1) {
        cursor->line = 1;
    }

    if (ctx->view->top_line_index < 1) {
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

struct cursor* nv_primary_cursor(struct nv_context* ctx)
{
    if (!ctx || !ctx->view) {
        return NULL;
    }

    return &ctx->view->cursors[NV_PRIMARY_CURSOR];
}
