#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#define TB_IMPL
#include "not-vim.h"
#include "editor.h"
#include "termbox2.h"

int main(int argc, char** argv) {
    assert(argc >= 2);
    int rv = 0;
    struct nv_editor editor = { 0 };
    nv_editor_init(&editor);

    if ((rv = tb_init()) != TB_OK) {
        fprintf(stderr, "%s\n", tb_strerror(rv));
        editor.status = rv;
        return rv;
    }

    struct nv_buff buff = {
        .name = argv[1],
        .type = NV_BUFFTYPE_SOURCE,
        .file = fopen(argv[1], "r+") // check this later
    };

    nv_push_buffer(&editor, &buff);

    editor.width = tb_width();
    editor.height = tb_height();
    editor.running = true;

    nv_render_term(&editor);
   
    tb_shutdown();
    free(editor.buffers);
    return rv;
}
