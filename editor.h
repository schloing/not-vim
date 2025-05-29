#ifndef NV_EDITOR_H
#define NV_EDITOR_H

#include <stdbool.h>
#define TUI
#include "buffer.h"
#include "cvector.h"
#include "window.h"

#define NV_INPUT_BACKLOG_CAP 5

typedef enum {
    NV_MODE_NAVIGATE,
    NV_MODE_INSERT,
    NV_MODE_HIGHLIGHT,
    NV_MODE_INSERTS, // insert*
} nv_mode;

struct nv_status {
    size_t height;
    char* format;
};

struct nv_conf {
    int bg;
    int fg;
    int tab_width;
    bool expand_tab;
    bool auto_indent;
    bool line_numbers;
    bool show_relative;
    int command_delay;
    int status_height;
    bool show_status;
    bool show_buffer;
    bool show_headless;
};

extern char* nv_mode_str[NV_MODE_INSERTS + 1];

struct nv_editor {
    struct nv_window* window;
    struct nv_window* logger;
    struct nv_status* statline;
    size_t active_window_index;
    nv_mode mode;
    size_t width;
    size_t height;
    int status;
    bool running;
    bool lua_loaded;
    char inputs[NV_INPUT_BACKLOG_CAP];
    struct nv_conf config;
};

// config defaults
#define NV_TAB_WIDTH 4
#define NV_EXPAND_TAB true
#define NV_AUTO_INDENT true
#define NV_LINE_NUMBERS true
#define NV_SHOW_RELATIVE true
#define NV_COMMAND_DELAY 350
#define NV_STATUS_HEIGHT 1
#define NV_SHOW_STATUS true
#define NV_SHOW_BUFFER true
#define NV_HEADLESS false

#if NV_HEADLESS
#undef TUI
#endif

void nv_resize_for_layout(struct nv_editor* editor, size_t width, size_t height);
void nv_main(struct nv_editor* editor);
int nv_editor_init(struct nv_editor* editor);
int nv_render_term(struct nv_editor* editor);
int nv_push_buffer(struct nv_editor* editor, struct nv_buff buffer);
int nv_open_window(struct nv_editor* editor, struct nv_window window);

#endif
