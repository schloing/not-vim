#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define TB_IMPL
#include "buffer.h"
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

    struct nv_buff* buff = nv_buffer_init(argv[1]);
    nv_push_buffer(&editor, *buff);
    editor.peek = buff->id;
   
    editor.width = tb_width();
    editor.height = tb_height();

    nv_mainloop(&editor);
   
    tb_shutdown();
    nv_free_buffers(&editor);
    return rv;
}
