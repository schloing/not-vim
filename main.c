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
#include "error.h"
#include "nvlua.h"
#define TB_IMPL
#include "termbox2.h"
#undef TB_IMPL
#include "color.h"
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

    editor.statline = &(struct nv_status){ .height = 1 };
    nv_resize_for_layout(&editor, tb_width(), tb_height());

    editor.logger = nv_find_empty_window(editor.window);
    editor.logger->buffer = calloc(1, sizeof(struct nv_buff));

    if ((rv = nv_buffer_init(editor.logger->buffer, NULL)) != NV_OK) {
        tb_shutdown();
        fprintf(stderr, "failed to create editor.logger: %s\n", nv_strerror(rv));
        return rv;
    }

    editor.logger->buffer->type = NV_BUFFTYPE_LOG;
    editor.logger->buffer->format = NV_FILE_FORMAT_PLAINTEXT;
    editor.logger->show = false;
    NV_WD_SET_SIZE(editor.logger->wd, editor.width, editor.height);

    // main window
    editor.window = nv_window_init();
    editor.window->split = HORIZONTAL;
    editor.window->show = true;
    NV_WD_SET_SIZE(editor.window->wd, editor.width, editor.height);

    // load plugs
    nvlua_main();

    for (int i = 1; i < argc; i++) {
        struct nv_window* window = nv_find_empty_window(editor.window);

        if (!window) {
            tb_shutdown();
            return -1;
        }

        if (window->parent) {
            (void)nv_redistribute(window->parent);
            window->split = window->parent->split == HORIZONTAL ? VERTICAL : HORIZONTAL;
        }

        if (window->show) {
            if (!window->buffer) {
                window->buffer = (struct nv_buff*)calloc(1, sizeof(struct nv_buff));
            }

            nv_buffer_init(window->buffer, argv[i]);
        }
    }

    nv_main(&editor);

clean_up:
    tb_shutdown();
    nv_free_windows(editor.logger);
    nv_free_windows(editor.window);
    return rv;
}
