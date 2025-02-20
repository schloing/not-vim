#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define CVECTOR_LOGARITHMIC_GROWTH

#include "buffer.h"
#include "cursor.h"
#include "cvector.h"
#include "editor.h"
#include "termbox2.h"
#include "window.h"

int main(int argc, char** argv) {
    int rv = 0;
    assert(argc >= 2);
    struct nv_editor editor = { 0 };
    nv_editor_init(&editor);

    if (!editor.nv_conf.show_headless && (rv = tb_init()) != TB_OK) {
        fprintf(stderr, "%s\n", tb_strerror(rv));
        editor.status = rv;
        return rv;
    }

    for (int i = 1; i < argc; i++) {
        struct nv_window* window = nv_find_empty_window(editor.window);
        if (!window) {
            tb_shutdown();
            return -1;
        }
        nv_open_window(&editor, *window);
    }

    editor.width = tb_width();
    editor.height = tb_height();

    nv_mainloop(&editor);

    tb_shutdown();
    nv_free_windows(editor.window);
    return rv;
}
