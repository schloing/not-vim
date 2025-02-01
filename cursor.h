#ifndef CURSOR_H
#define CURSOR_H

#include "bufferhelp.h"

struct cursor {
    int x;
    int y;
    int line;
    char ch;
    size_t buff_id;
};

#define NV_CURSOR_CAP       5
#define NV_PRIMARY_CURSOR   0

struct cursor* nv_primary_cursor(struct nv_buff_internal* buffer);
void nv_cursor_insert_ch(struct nv_buff_internal* buffer, struct cursor* cursor, char ch);
void nv_cursor_move_down(struct nv_buff_internal* buffer, struct cursor* cursor, int amt);
void nv_cursor_move_left(struct nv_buff_internal* buffer, struct cursor* cursor, int amt);
void nv_cursor_move_right(struct nv_buff_internal* buffer, struct cursor* cursor, int amt);
void nv_cursor_move_up(struct nv_buff_internal* buffer, struct cursor* cursor, int amt);

#endif
