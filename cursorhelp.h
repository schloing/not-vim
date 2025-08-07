#ifndef NV_CURSOR_HELP
#define NV_CURSOR_HELP

#include "editor.h"

struct cursor* nv_primary_cursor(struct nv_buff* buffer);
void nv_cursor_insert_ch(struct nv_view* view, struct cursor* cursor, char ch);
void nv_cursor_move_down(struct nv_view* view, struct cursor* cursor, int amt);
void nv_cursor_move_left(struct nv_view* view, struct cursor* cursor, int amt);
void nv_cursor_move_right(struct nv_view* view, struct cursor* cursor, int amt);
void nv_cursor_move_up(struct nv_view* view, struct cursor* cursor, int amt);

#endif
