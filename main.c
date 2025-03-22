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

int main(int argc, char** argv)
{
    int rv = 0;
    assert(argc >= 2);
    struct nv_editor editor = { 0 };
    nv_editor_init(&editor);

    if (!editor.nv_conf.show_headless && (rv = tb_init()) != TB_OK) {
        fprintf(stderr, "%s\n", tb_strerror(rv));
        editor.status = rv;
        goto clean_up;
    }

    editor.width = tb_width();
    editor.height = tb_height();
    editor.window = nv_window_init();
    editor.window->wd.w = editor.width;
    editor.window->wd.h = editor.height;

    for (int i = 1; i < argc; i++) {
        struct nv_window* window = nv_find_empty_window(editor.window);

        if (!window) {
            tb_shutdown();
            return -1;
        }

        if (window->parent) {
            nv_redistribute(window->parent);
        }

        if (window->show) {
            if (!window->buffer) {
                window->buffer = (struct nv_buff*)calloc(1, sizeof(struct nv_buff));
            }

            nv_buffer_init(window->buffer, argv[i]);
        }
    }

    nv_mainloop(&editor);

clean_up:
    tb_shutdown();
    nv_free_windows(editor.window);
    return rv;
}
