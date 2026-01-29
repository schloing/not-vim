#include "nvtree/nvtree.h"
#include <assert.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define CVECTOR_LOGARITHMIC_GROWTH

#include "buffer.h"
#include "color.h"
#include "status.h"
#include "editor.h"
#include "error.h"
#include "nvlua.h"
#define TB_IMPL
#include "termbox2.h"
#undef TB_IMPL
#include "window.h"

static int nv_open_file_in_window(struct nv_editor* editor, struct nv_window_node* root, const char* filename);
static void nv_fatal_signal(int sig, siginfo_t* info, void* ucontext);
static void nv_editor_cleanup(struct nv_editor* editor);
static void nv_cleanup();
static void nv_setup_signal_handlers();
static void nv_must_be_no_errors(const char* message);

static int nv_open_file_in_window(struct nv_editor* editor, struct nv_window_node* root, const char* filename)
{
    struct nv_window_node* window = nv_window_node_init();

    if (!window) {
        return NV_ERR_MEM;
    }

    // window->leaf.view = nv_view_init(filename);

    if (nv_editor->status != NV_OK) {
        return NV_ERR_MEM;
    }

    (void)nv_window_node_push_child(root, window);
    editor->focus = window;

    return editor->status;
}

static void nv_editor_cleanup(struct nv_editor* editor)
{
    tb_shutdown();
    nv_free_windows(editor->logger);
    nv_free_windows(editor->window);
}

static void nv_cleanup()
{
    if (nv_editor) {
        nv_editor_cleanup(nv_editor);
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
    atexit(nv_cleanup);

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

static void nv_must_be_no_errors(const char* message)
{
    if (nv_editor->status != NV_OK) {
        nv_fatal(message);
        exit(nv_editor->status);
    }
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

//  editor.statline = &(struct nv_status){ .height = 1 };
//  nv_resize_for_layout(tb_width(), tb_height());

    editor->window = nv_window_node_init();
    nv_must_be_no_errors("failed to create editor base window");

    editor.logger = nv_window_node_init();
    nv_must_be_no_errors("failed to create log window");
    editor.logger->leaf.view = nv_view_init(NULL);
    nv_must_be_no_errors("failed to create log buffer");

    struct nv_context logger_ctx = nv_get_context(editor.logger);
    logger_ctx.buffer->type = NV_BUFF_TYPE_LOG;
    logger_ctx.buffer->format = NV_FILE_FORMAT_PLAINTEXT;
    logger_ctx.view->visible = false;

    struct nv_window_node* primary = nv_window_node_init();
    nv_must_be_no_errors("failed to create primary window");
    primary = nv_window_node_push_child(editor.window, primary);

    assert(editor.window->kind == NV_WM_SPLIT);
    editor.window->split.kind = NV_SPLIT_HORIZONTAL;
    editor.window->split.ratio = 0.1;

    for (int i = 1; i < argc; i++) {
        if (nv_open_file_in_window(&editor, primary, (const char*)argv[i]) != NV_OK) {
            nv_fatal("failed to open a file");
            exit(editor.status);
        }
    }

    struct nv_window_node* status = nv_window_node_init();
    nv_must_be_no_errors("failed to create status window");
    status->leaf.view = nv_view_init(NULL);
    nv_must_be_no_errors("failed to create status view");

    status->leaf.view->allow_split = false;
    status = nv_window_node_push_child(editor.window, status);

#ifndef NV_NO_LUAJIT
    // load plugs
    nvlua_main();
#endif

    nv_main();

    nv_editor_cleanup(&editor);
    return editor.status;
}
