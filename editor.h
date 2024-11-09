#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#define TUI
#include "vec.h"

typedef struct nv_buff* nv_buff_vec;

struct nv_editor {
    // nv_buff*
    nv_buff_vec buffers;
    size_t peek;

    // rendering tui
#ifdef TUI
    int  width;
    int  height;
#endif 

    // editor status
    int  status;
    bool running;

    // config
    struct nv_conf {
         int  tab_width;
         bool expand_tab;
         bool auto_indent;
         bool line_numbers;
         bool show_relative;
         int  command_delay;
         int  status_height;
    } nv_conf;
};

// config defaults
#define NV_TAB_WIDTH     4
#define NV_EXPAND_TAB    true
#define NV_AUTO_INDENT   true
#define NV_LINE_NUMBERS  true
#define NV_SHOW_RELATIVE true
#define NV_COMMAND_DELAY 350
#define NV_STATUS_HEIGHT 1

void nv_editor_init(struct nv_editor* editor);
void nv_render_term(struct nv_editor* editor);
void nv_push_buffer(struct nv_editor* editor, struct nv_buff buffer);
void nv_mainloop(struct nv_editor* editor);

#endif
