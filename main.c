#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define TB_IMPL
#include "not-vim.h"
#include "editor.h"
#include "termbox2.h"
#include "vec.h"

int main(int argc, char** argv) {
    int rv = 0;
    assert(argc >= 2);
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
        .file = fopen(argv[1], "r+"),
    };

    nv_push_buffer(&editor, buff);
    editor.peek = 0;
   
    editor.width = tb_width();
    editor.height = tb_height();

    nv_mainloop(&editor);
   
    tb_shutdown();
    vector_free(editor.buffers);
    return rv;
}
