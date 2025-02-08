#include "buffer.h"
#include "cursor.h"
#include "termbox2.h"

void nv_cursor_insert_ch(struct nv_buff* buffer, struct cursor* cursor, char ch) {
    struct nv_buff_line* l = line(buffer, cursor->line);
    size_t row = l->begin + cursor->x;

    cvector_insert(buffer->buffer, row, ch);
    cvector_clear(buffer->lines); // FIXME
    nv_load_file_buffer(buffer, &buffer->line_count);

    cursor->x++;
}

void nv_cursor_move_down(struct nv_buff* buffer, struct cursor* cursor, int amt) {
    if (cursor->y < (int)buffer->line_count) {
        cursor->y++;
        cursor->line++;
    }
}

void nv_cursor_move_up(struct nv_buff* buffer, struct cursor* cursor, int amt) {
    if (cursor->y > 0) {
        cursor->y--;
        cursor->line--;
    }
}

void nv_cursor_move_left(struct nv_buff* buffer, struct cursor* cursor, int amt) {
    cursor->x--;
}

void nv_cursor_move_right(struct nv_buff* buffer, struct cursor* cursor, int amt) {
    cursor->x++;
}

struct cursor* nv_primary_cursor(struct nv_buff* buffer) {
    return &buffer->cursors[NV_PRIMARY_CURSOR];
}
