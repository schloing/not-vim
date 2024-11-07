#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "termbox2.h"
#include "not-vim.h"
#include "editor.h"

void nv_editor_init(struct nv_editor* editor) {
    assert(editor);

    editor->len = 0;
    editor->cap = 10;
    editor->buffers = (struct nv_buff*)calloc(editor->cap, sizeof(struct nv_buff*));

    editor->nv_conf = (struct nv_conf){
        .tab_width     = NV_TAB_WIDTH,
    	.expand_tab    = NV_TAB_WIDTH,
    	.auto_indent   = NV_AUTO_INDENT,
    	.line_numbers  = NV_LINE_NUMBERS,
    	.show_relative = NV_SHOW_RELATIVE,
    	.command_delay = NV_COMMAND_DELAY,
    };
}

void nv_push_buffer(struct nv_editor* editor, struct nv_buff* buffer) {
    size_t id = editor->len;
    buffer->id = id;
    editor->buffers[id] = *buffer;
    if (id >= editor->cap) return; // realloc
}

static void _nv_status(struct nv_editor* editor);
static void _nv_emulator();

void nv_render_term(struct nv_editor* editor) {
    while (editor->running) {
        static int i = 0;
        _nv_status(editor);
        tb_present();
        if (i++ >= 5) editor->running = false;
        sleep(1);
    }
}

static void _nv_status(struct nv_editor* editor) {
    tb_print(0, editor->height - 1, TB_WHITE, TB_BLACK, "sN+editor.c");
}
