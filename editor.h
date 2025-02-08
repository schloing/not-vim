#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#define TUI
#include "buffer.h"
#include "window.h"
#include "cvector.h"

#define INPUT_BACKLOG_CAP 5

typedef enum {
    NV_MODE_NAVIGATE,
    NV_MODE_INSERT,
    NV_MODE_HIGHLIGHT,
    NV_MODE_INSERTS, // insert*
} nv_mode;

extern char* nv_mode_str[NV_MODE_INSERTS + 1];

struct nv_editor {
    cvector(struct nv_window) buffers;
    struct nv_buff* current;
#ifdef TUI                   // if there is ever a qt/gui version
    int         width;
    int         height;
#endif
    nv_mode     mode;
    int         status;
    bool        running;
    char        inputs[INPUT_BACKLOG_CAP];
    struct nv_conf nv_conf;  // default config for buffers
};

// config defaults
#define NV_TAB_WIDTH     4
#define NV_EXPAND_TAB    true
#define NV_AUTO_INDENT   true
#define NV_LINE_NUMBERS  true
#define NV_SHOW_RELATIVE true
#define NV_COMMAND_DELAY 350
#define NV_STATUS_HEIGHT 1
#define NV_SHOW_STATUS   true
#define NV_SHOW_BUFFER   true
#define NV_HEADLESS      false

#if NV_HEADLESS
#undef TUI
#endif

void nv_editor_init(struct nv_editor* editor);
void nv_render_term(struct nv_editor* editor);
void nv_mainloop(struct nv_editor* editor);
void nv_push_buffer(struct nv_editor* editor, struct nv_buff buffer);
void nv_free_buffers(struct nv_editor* editor);

#endif
