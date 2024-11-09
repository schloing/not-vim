#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "editor.h"
#include "not-vim.h"
#include "termbox2.h"
#include "vec.h"

static void _nv_status(struct nv_editor* editor);
static struct nv_buff* _nv_get_active_buffer(struct nv_editor* editor);

void nv_editor_init(struct nv_editor* editor) {
    assert(editor);

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

void nv_push_buffer(struct nv_editor* editor, struct nv_buff buffer) {
    buffer.id = vector_size(editor->buffers);
    vector_add(&editor->buffers, buffer);
}

void nv_mainloop(struct nv_editor* editor) {
    tb_set_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);
    struct tb_event ev;
    editor->running = true;

    while (editor->running) {
        editor->status = tb_poll_event(&ev);

        if (editor->status != TB_OK) {
            if (editor->status == TB_ERR_POLL &&
                tb_last_errno() == EINTR) {
                /* poll was interrupted, maybe by a SIGWINCH; try again */
                continue;
            }
            /* some other error occurred; bail */
            break;
        }
        
        tb_clear();

        switch (ev.type) {
        case TB_EVENT_KEY:
            if (ev.key == TB_KEY_) {
                editor->status = -1;
                return;
            }

            tb_printf(0, 0, TB_WHITE, TB_BLACK, "%d", ev.key);
            break;
        case TB_EVENT_RESIZE:
            break;
        case TB_EVENT_MOUSE:
            break;
        default: break;
        }

        _nv_status(editor);
        
        tb_present();
        sleep(1);
    }
}

void nv_render_term(struct nv_editor* editor) {
    // TODO
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
    struct nv_buff* active = _nv_get_active_buffer(editor);
    tb_printf(0, editor->height - 1, TB_WHITE, TB_BLACK, "[%d]%s", active->id, active->name);
}
