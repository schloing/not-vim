#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "assert.h"
#include "buffer.h"
#include "editor.h"
#include "termbox2.h"
#include "vec.h"

static void _nv_status(struct nv_editor* editor);
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
    struct tb_event ev;
    editor->running = true;
    while (editor->running) {
        editor->status = tb_peek_event(&ev, 100);
        tb_clear();

        switch (ev.type) {
        case TB_EVENT_KEY:
            editor->status = ev.key ? ev.key : ev.ch;
            if (ev.key == TB_KEY_ESC) return;
#define NV_BUFFER_INSERT_CHAR(editor, character)
            NV_BUFFER_INSERT_CHAR(editor, ev.ch);
            break;

        default: break;
        }

        _nv_status(editor);
        tb_present();
    }
}

void nv_push_buffer(struct nv_editor* editor, struct nv_buff buffer) {
    buffer.id = vector_size(editor->buffers);
    vector_add(&editor->buffers, buffer);
}

void nv_render_term(struct nv_editor* editor) {
    // TODO:
    // terminal integration
}

static struct nv_buff*
_nv_get_active_buffer(struct nv_editor* editor) {
    size_t index = editor->peek;
    struct nv_buff* rv = malloc(sizeof(struct nv_buff));
    struct nv_buff active = (struct nv_buff)editor->buffers[index];
    memcpy(rv, &active, sizeof(struct nv_buff));
    return rv;
}

static void
_nv_status(struct nv_editor* editor) {
    struct nv_buff* active_buffer = _nv_get_active_buffer(editor);
    tb_printf(0, editor->height - 1, TB_WHITE, TB_BLACK, "[%zu %d]%s",
                editor->status, active_buffer->id, active_buffer->path);
}
