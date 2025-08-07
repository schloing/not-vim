#include "cursor.h"
#include "cursorhelp.h"
#include "buffer.h"
#include "termbox2.h"

void nv_cursor_insert_ch(struct nv_view* view, struct cursor* cursor, char ch)
{
    struct nv_buff_line* l = line(view, cursor->line);
    size_t row = l->begin + cursor->x;

    cvector_insert(view->buffer->buffer, row, ch);
    cvector_clear(view->buffer->lines); // FIXME
    nv_load_file_buffer(view->buffer, &view->line_count);

    cursor->x++;
}

void nv_cursor_move_down(struct nv_view* view, struct cursor* cursor, int amt)
{
    if (cursor->y + amt < (int)view->line_count) {
        cursor->y += amt;
        cursor->line += amt;
    }
}

void nv_cursor_move_up(struct nv_view* view, struct cursor* cursor, int amt)
{
    if (cursor->y - amt >= 0) {
        cursor->y -= amt;
        cursor->line -= amt;
    }
}

void nv_cursor_move_left(struct nv_view* view, struct cursor* cursor, int amt)
{
    int length = (int)line(view, cursor->line)->length;

    if (cursor->x > length)
        cursor->x = length;

    cursor->x -= amt;

    if (cursor->x < 0)
        cursor->x = 0;
}

void nv_cursor_move_right(struct nv_view* view, struct cursor* cursor, int amt)
{
    int length = (int)line(view, cursor->line)->length;

    cursor->x += amt;

    if (cursor->x > length)
        cursor->x = length;
}

struct cursor* nv_primary_cursor(struct nv_buff* buffer)
{
    return &buffer->cursors[NV_PRIMARY_CURSOR];
}
