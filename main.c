#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define CVECTOR_LOGARITHMIC_GROWTH

#include "buffer.h"
#include "cursor.h"
#include "color.h"
#include "cvector.h"
#include "status.h"
#include "editor.h"
#include "error.h"
#include "nvlua.h"
#define TB_IMPL
#include "termbox2.h"
#undef TB_IMPL
#include "window.h"

static int nv_open_file_in_window(struct nv_editor* editor, struct nv_window* root, const char* filename);
static void nv_editor_cleanup(struct nv_editor* editor);

static int nv_open_file_in_window(struct nv_editor* editor, struct nv_window* root, const char* filename)
{
    struct nv_window* window = nv_create_child_window(root);

    if (!window) {
        return NV_ERR_NOT_INIT;
    }

    (void)nv_redistribute(window->parent);

    if (!window->view) {
        window->view = nv_view_init(filename);
    }

    return nv_editor->status;
}

static void nv_editor_cleanup(struct nv_editor* editor)
{
    tb_shutdown();
    nv_free_windows(editor->logger);
    nv_free_windows(editor->window);

    if (editor->statline && editor->statline->format) {
        free(editor->statline->format);
    }
}

int main(int argc, char** argv)
{
    assert(argc >= 2);
    struct nv_editor editor = { 0 };
    nv_editor_init(&editor);

    if (!editor.config.show_headless && (editor.status = tb_init()) != TB_OK) {
        fprintf(stderr, "%s\n", tb_strerror(editor.status));
        nv_editor_cleanup(&editor);
        return editor.status;
    }

    editor.statline = &(struct nv_status){ .height = 1 };
    nv_resize_for_layout(tb_width(), tb_height());

    editor.logger = nv_window_init();

    if (editor.status != NV_OK) {
        nv_fatal("failed to create log window");
        nv_editor_cleanup(&editor);
        return editor.status;
    }

    editor.logger->view = nv_view_init(NULL);

    if (editor.status != NV_OK) {
        nv_fatal("failed to create log buffer");
        nv_editor_cleanup(&editor);
        return editor.status;
    }

    editor.logger->view->buffer->type = NV_BUFF_TYPE_LOG;
    editor.logger->view->buffer->format = NV_FILE_FORMAT_PLAINTEXT;
    editor.logger->show = false;
    NV_WD_SET_SIZE(editor.logger->wd, editor.width, editor.height);

    if (editor.status != NV_OK) {
        nv_fatal("failed to create editor window");
        nv_editor_cleanup(&editor);
        return editor.status;
    }

    // main window
    editor.window = nv_window_init();
    editor.window->split = HORIZONTAL;
    editor.window->show = true;
    NV_WD_SET_SIZE(editor.window->wd, editor.width, editor.height);

    // load plugs
    nvlua_main();

    for (int i = 1; i < argc; i++) {
        if (nv_open_file_in_window(&editor, editor.window, (const char*)argv[i]) != NV_OK) {
            nv_fatal("failed to open a file");
            nv_editor_cleanup(&editor);
            return editor.status;
        }
    }

    nv_main();

    nv_editor_cleanup(&editor);
    return editor.status;
}
