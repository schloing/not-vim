#ifndef NV_EDITOR_H
#define NV_EDITOR_H

#include <stdbool.h>
#include <stdint.h>
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
    int      tab_width;
    bool     expand_tab;
    bool     auto_indent;
    bool     line_numbers;
    bool     show_relative;
    int      command_delay;
    int      status_height;
    bool     show_status;
    bool     show_buffer;
    bool     show_headless;
    bool     allow_italic_text;
    bool     allow_bolded_text;
    bool     allow_underlined_text;
    uint32_t fg_main;
    uint32_t fg_secondary;
    uint32_t fg_contrast;
    uint32_t bg_main;
    uint32_t bg_secondary;
    uint32_t bg_contrast;
    uint32_t fg_syntax_keyword;
    uint32_t fg_syntax_string;
    uint32_t fg_syntax_comment;
    uint32_t fg_syntax_number;
    uint32_t fg_syntax_operator;
    uint32_t fg_syntax_function;
    uint32_t fg_syntax_variable;
    uint32_t fg_syntax_type;
    uint32_t fg_syntax_macro;
    uint32_t fg_syntax_preprocessor;
    uint32_t fg_syntax_constant;
    uint32_t fg_syntax_builtins;
    uint32_t fg_syntax_attribute;
    uint32_t fg_syntax_error;
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

// defaults

#define NV_TAB_WIDTH               4
#define NV_EXPAND_TAB              true
#define NV_AUTO_INDENT             true
#define NV_LINE_NUMBERS            true
#define NV_SHOW_RELATIVE           true
#define NV_COMMAND_DELAY           350      // ms
#define NV_STATUS_HEIGHT           1
#define NV_SHOW_STATUS             true
#define NV_SHOW_BUFFER             true
#define NV_HEADLESS                false
#define NV_ALLOW_ITALIC_TEXT       true
#define NV_ALLOW_BOLDED_TEXT       true
#define NV_ALLOW_UNDERLINED_TEXT   true

// config defaults to 32-bit colours regardless of support for it
// the editor does not check to see whether the colours are supported or not

#define NV_FG_MAIN                 0xdddddd  // light gray
#define NV_FG_SECONDARY            0xaaaaaa  // dimmer gray
#define NV_FG_CONTRAST             0xffffff  // white
#define NV_BG_MAIN                 0x1e1e1e  // dark background
#define NV_BG_SECONDARY            0x2e2e2e  // slightly lighter background
#define NV_BG_CONTRAST             0x000000  // black

#define NV_FG_SYNTAX_KEYWORD       0x569cd6  // blue
#define NV_FG_SYNTAX_STRING        0xd69d85  // light red/orange
#define NV_FG_SYNTAX_COMMENT       0x6a9955  // green
#define NV_FG_SYNTAX_NUMBER        0xb5cea8  // greenish
#define NV_FG_SYNTAX_OPERATOR      0xd4d4d4  // near white
#define NV_FG_SYNTAX_FUNCTION      0x61afef  // light blue
#define NV_FG_SYNTAX_VARIABLE      0xe06c75  // reddish
#define NV_FG_SYNTAX_TYPE          0x4ec9b0  // teal
#define NV_FG_SYNTAX_MACRO         0xffc66d  // yellow/orange
#define NV_FG_SYNTAX_PREPROCESSOR  0x9cdcfe  // light blue
#define NV_FG_SYNTAX_CONSTANT      0xd7ba7d  // beige/yellow
#define NV_FG_SYNTAX_BUILTINS      0x56b6c2  // cyan
#define NV_FG_SYNTAX_ATTRIBUTE     0xce9178  // orange
#define NV_FG_SYNTAX_ERROR         0xff5370  // bright red

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
