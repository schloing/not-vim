#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "nv.h"

void editor_init(struvt nv_editor* editor) {
    assert(editor);
    editor->buffers = (nv_bufF*)malloc(sizeof(nv_buff) * editor->cap);
    
    editor->nv_conf.tab_width     = NV_TAB_WIDTH;
    editor->nv_conf.expand_tab    = NV_TAB_WIDTH;
    editor->nv_conf.auto_indent   = NV_AUTO_INDENT;
    editor->nv_conf.line_numbers  = NV_LINE_NUMBERS;
    editor->nv_conf.show_relative = NV_SHOW_RELATIVE;
    editor->nv_conf.command_delay = NV_COMMAND_DELAY;
}
