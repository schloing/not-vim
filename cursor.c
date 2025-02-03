#include "buffer.h"
#include "cursor.h"
#include "termbox2.h"

size_t nv_get_line_length(struct nv_buff_line line) {
    int end_of_line = line.end - line.begin - 1;
    return end_of_line > 0 ? end_of_line : 0;
}

void nv_cursor_insert_ch(struct nv_buff* buffer, struct cursor* cursor, char ch) {
    struct nv_buff_line* l = line(buffer, cursor->line);
    size_t row = l->begin + cursor->x;

    cvector_insert(buffer->buffer, row, ch);
    cvector_clear(buffer->lines); // FIXME
    nv_load_file_buffer(buffer, &buffer->_line_count);

    cursor->x++;
}

static void _nv_clamp_x(struct nv_buff_line* line, struct cursor* cursor) {
    if (cursor->x > line->length) {
        cursor->xdraw = line->length;
        return;
    }

    cursor->xdraw = cursor->x;
}

void nv_cursor_move_down(struct nv_buff* buffer, struct cursor* cursor, int amt) {
    if (cursor->y <= (size_t)tb_height() - 3 && cursor->line < buffer->_line_count) {
        cursor->line++;
        cursor->y++;
    } else if (cursor->line >= buffer->_line_count) {
        cursor->line = buffer->_line_count;
        // cursor->y same
    } else {
        cursor->line++;
        buffer->_begin_line++;
    }

    _nv_clamp_x(currline(buffer), cursor);
}

void nv_cursor_move_up(struct nv_buff* buffer, struct cursor* cursor, int amt) {
    if (cursor->y > 0 && cursor->line > 0) {
        cursor->line--;
        cursor->y--;
    } else if (cursor->line <= 0) {
        cursor->line = 0;
        // cursor->y same
    } else {
        cursor->line--;
        buffer->_begin_line--;
    }

    _nv_clamp_x(currline(buffer), cursor);
}

void nv_cursor_move_left(struct nv_buff* buffer, struct cursor* cursor, int amt) {
    if (cursor->x > 0)
        cursor->x--;

    // cursor->xtmp = cursor->x;
}

void nv_cursor_move_right(struct nv_buff* buffer, struct cursor* cursor, int amt) {
    struct nv_buff_line* line = currline(buffer);

    if (cursor->x < nv_get_line_length(*line)) 
        cursor->x++;
    
    // cursor->xtmp = cursor->x;
}

struct cursor* nv_primary_cursor(struct nv_buff* buffer) {
    return &buffer->cursors[NV_PRIMARY_CURSOR];
}
