#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "assert.h"
#include "buffer.h"
#include "editor.h"
#include "termbox2.h"
#include "vec.h"

static void _nv_inputloop(struct nv_editor* editor);
static void _nv_draw_buffer(struct nv_editor* editor);
static void _nv_draw_status(struct nv_editor* editor);
static struct nv_buff* _nv_get_active_buffer(struct nv_editor* editor);

void nv_editor_init(struct nv_editor* editor) {
    NV_ASSERT(editor);

    editor->buffers = vector_create();
    
    editor->nv_conf = (struct nv_conf){
        .tab_width     = NV_TAB_WIDTH,
        .expand_tab    = NV_TAB_WIDTH,
        .auto_indent   = NV_AUTO_INDENT,
        .line_numbers  = NV_LINE_NUMBERS,
        .show_relative = NV_SHOW_RELATIVE,
        .command_delay = NV_COMMAND_DELAY,
    };
}

void nv_mainloop(struct nv_editor* editor) {
    tb_set_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);

    _nv_draw_buffer(editor);
    _nv_draw_status(editor);
    tb_present();

    editor->running = true;
    _nv_inputloop(editor);
}

static void
_nv_inputloop(struct nv_editor* editor) {
    struct tb_event ev;

    while (editor->running) {
        editor->status = tb_poll_event(&ev);

        switch (ev.type) {
        case TB_EVENT_KEY:
            editor->status = ev.key ? ev.key : ev.ch;
            if (ev.key == TB_KEY_ESC) return;
#define NV_BUFFER_INSERT_CHAR(editor, character)
            NV_BUFFER_INSERT_CHAR(editor, ev.ch);
            break;

        case TB_EVENT_RESIZE:
            editor->height = tb_height();
            editor->width = tb_width();
            editor->buffers[editor->peek].loaded = false; // redraw lines
            tb_clear();
            _nv_draw_buffer(editor);
            _nv_draw_status(editor);
            tb_present();
            break;

        default: break;
        }
    }
}

void nv_push_buffer(struct nv_editor* editor, struct nv_buff buffer) {
    buffer.id = vector_size(editor->buffers);
    vector_add(&editor->buffers, buffer);
}

static struct nv_buff*
_nv_get_active_buffer(struct nv_editor* editor) {
    size_t index = editor->peek;
    // why the fuck was i allocating a new buffer here
    // fixed
    struct nv_buff* active = (struct nv_buff*)&editor->buffers[index];
    return active;
}

static void
_nv_draw_buffer(struct nv_editor* editor) {
    struct nv_buff* buffer = _nv_get_active_buffer(editor);
    switch (buffer->type) {
    case NV_BUFFTYPE_SOURCE:
        if (!buffer->loaded) _nv_load_file_buffer(buffer);
        break;

    default:
        fprintf(stderr, "unsupported bufftype %d\n", buffer->type);
        break;
    }
}

static void
_nv_draw_status(struct nv_editor* editor) {
    struct nv_buff* active_buffer = _nv_get_active_buffer(editor);
    char* prompt;
    asprintf(&prompt, "[%zu]%s", active_buffer->id, active_buffer->path);
    tb_printf(0, editor->height - 1, TB_WHITE, TB_BLACK, "%-*.*s", editor->width, editor->width, prompt);
    free(prompt);
}
