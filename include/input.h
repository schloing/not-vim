#ifndef NV_INPUT_H
#define NV_INPUT_H

#include "editor.h"
#include "termbox2.h"

void nv_handle_mouse_input(struct tb_event* ev);
void nv_handle_key_input(unsigned char ansi);
void nv_set_mode(nv_mode mode);

#endif