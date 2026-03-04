#ifndef NV_DRAW_H
#define NV_DRAW_H

#include "window.h"
#include "view.h"

void nv_draw_cursor();
int nv_draw_windows(struct nv_window_node* root, const struct nv_window_area area);
int nv_draw_text_buffer(struct nv_view* view, const struct nv_window_area* area);
int nv_draw_view(struct nv_view* view, const struct nv_window_area* area);
void nv_draw_background_rect(int x1, int y1, int x2, int y2);
void nv_draw_background();
void nv_buffer_printf(struct nv_view* view, const struct nv_window_area* area, int row, int line_no, char* lbuf, size_t length);
int nv_calculate_statline();

#endif