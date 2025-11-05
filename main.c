#include <assert.h>
#include <dlfcn.h>
#include <execinfo.h>
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

static void nv_fatal_signal(int sig, siginfo_t* info, void* ucontext)
{
    (void)ucontext;
    tb_shutdown();

    const char* sig_name;

    switch(sig) {
        case SIGSEGV: sig_name = "SIGSEGV"; break;
        case SIGBUS:  sig_name = "SIGBUS";  break;
        case SIGFPE:  sig_name = "SIGFPE";  break;
        case SIGABRT: sig_name = "SIGABRT"; break;
        case SIGILL:  sig_name = "SIGILL"; break;
        case SIGTRAP: sig_name = "SIGTRAP"; break;
        default:      sig_name = "UNKNOWN"; break;
    }

    fprintf(stderr, "%s (%d) at address %p\n", sig_name, sig, info->si_addr);

    void* bt[32];
    int bt_size = backtrace(bt, 32);
    backtrace_symbols_fd(bt, bt_size, STDERR_FILENO);

    exit(128 + sig);
}

static void nv_setup_signal_handlers()
{
    signal(SIGTTOU, SIG_IGN);

    struct sigaction sa;
    sa.sa_sigaction = nv_fatal_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESETHAND;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS,  &sa, NULL);
    sigaction(SIGFPE,  &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
}

int main(int argc, char** argv)
{
    nv_setup_signal_handlers();

    assert(argc >= 2);
    struct nv_editor editor = { 0 };

    if (nv_editor_init(&editor) != NV_OK) {
        nv_editor_cleanup(&editor);
        return editor.status;
    }

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

    if (editor.status != NV_OK || !editor.logger->view->buffer) {
        nv_fatal("failed to create log buffer");
        nv_editor_cleanup(&editor);
        return editor.status;
    }

    editor.logger->view->buffer->type = NV_BUFF_TYPE_LOG;
    editor.logger->view->buffer->format = NV_FILE_FORMAT_PLAINTEXT;
    editor.logger->show = false;

    nv_window_set_dim(editor.logger, 1, 1);

    if (editor.status != NV_OK) {
        nv_fatal("failed to create editor window");
        nv_editor_cleanup(&editor);
        return editor.status;
    }

    // main window
    editor.window = nv_window_init();
    editor.window->split = HORIZONTAL;
    editor.window->show = true;

    nv_window_set_dim(editor.window, 1, 1);

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
