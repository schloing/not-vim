#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "editor.h"
#include "general.h"
#include "not-vim.h"
#include "termbox2.h"

static void _nv_status(struct nv_editor* editor);
static struct nv_buff* _nv_get_active_buffer(struct nv_editor* editor);

void nv_editor_init(struct nv_editor* editor) {
    assert(editor);

    editor->buffers = new_vector(10 * sizeof(struct nv_buff));

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
    buffer->id = editor->buffers->index;
    push_vector(editor->buffers, buffer);
}

void nv_render_term(struct nv_editor* editor) {
    while (editor->running) {
        static int i = 0;
        _nv_status(editor);
        tb_present();
        if (i++ >= 5) editor->running = false;
        sleep(1);
    }
}

static struct nv_buff*
_nv_get_active_buffer(struct nv_editor* editor) {
    size_t index = editor->buffers->peek;
    struct nv_buff* active = &((struct nv_buff*)editor->buffers->data)[index];
    return active;
}

static void _nv_status(struct nv_editor* editor) {
    struct nv_buff* active = _nv_get_active_buffer(editor);
    tb_printf(0, editor->height - 1, TB_WHITE, TB_BLACK, "[%d]%s", active->id, active->name);
}
