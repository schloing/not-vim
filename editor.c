#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "buffer.h"
#include "color.h"
#include "cvector.h"
#include "draw.h"
#include "editor.h"
#include "events.h"
#include "input.h"
#include "error.h"
#include "nvapi.h"
#include "termbox2.h"
#include <uv.h>
#include "view.h"
#include "window.h"

static int nv_get_input(struct tb_event* ev);
static void nv_redraw_all();
static void nv_close_pollers();
static void nv_on_tty(uv_poll_t* handle, int status, int events);
static void nv_on_tty_stream(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
static void nv_tty_stream_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
static void nv_register_pollers(uv_loop_t* loop, struct nv_poller_fd fds[], size_t nfds);
static void nv_on_nng_recv(uv_poll_t* handle, int status, int events);
static void nv_on_nng_send(uv_poll_t* handle, int status, int events);
static void nv_shutdown();
static void nv_on_poller_closed(uv_handle_t* handle);
static void nv_on_tty_closed(uv_handle_t* handle);


_Thread_local struct nv_editor* nv_editor = NULL; // extern in editor.h 

// extern in editor.h
char* nv_mode_str[NV_MODE_INSERTS + 1] = {
    "NAV",
    "INS",
    "HIGH",
    "INS*",
};

// extern in buffer.h
char* nv_str_buff_type[NV_BUFF_TYPE_END] = {
    "stdin",
    "stdout",
    "browser",
    "network",
    "source",
    "plaintext",
    "log",
};

// extern in buffer.h
char* nv_str_buff_fmt[NV_FILE_FORMAT_END] = {
    "binary",
    "source",
    "plaintext",
};

int nv_editor_init(struct nv_editor* editor)
{
    if (!nv_editor) {
        nv_editor = editor;
    }

    nv_editor->mode = (nv_mode)NV_MODE_NAVIGATE;
    clock_gettime(CLOCK_MONOTONIC, &nv_editor->start);
#define NV_WINDOW_CAP 16
    cvector_reserve(nv_editor->windows, (size_t)NV_WINDOW_CAP);
    cvector_reserve(nv_editor->views, (size_t)NV_WINDOW_CAP);

    nv_editor->config = (struct nv_conf) {
        .tab_width              = NV_TAB_WIDTH,
        .expand_tab             = NV_EXPAND_TAB,
        .auto_indent            = NV_AUTO_INDENT,
        .line_numbers           = NV_LINE_NUMBERS,
        .show_relative          = NV_SHOW_RELATIVE,
        .command_delay          = NV_COMMAND_DELAY,
        .status_height          = NV_STATUS_HEIGHT,
        .show_status            = NV_SHOW_STATUS,
        .show_buffer            = NV_SHOW_BUFFER,
        .show_headless          = NV_HEADLESS,
        .allow_italic_text      = NV_ALLOW_ITALIC_TEXT,
        .allow_bolded_text      = NV_ALLOW_BOLDED_TEXT,
        .allow_underlined_text  = NV_ALLOW_UNDERLINED_TEXT,
        .fg_main                = NV_FG_MAIN,
        .fg_secondary           = NV_FG_SECONDARY,
        .fg_contrast            = NV_FG_CONTRAST,
        .bg_main                = NV_BG_MAIN,
        .bg_secondary           = NV_BG_SECONDARY,
        .bg_contrast            = NV_BG_CONTRAST,
        .fg_syntax_keyword      = NV_FG_SYNTAX_KEYWORD,
        .fg_syntax_string       = NV_FG_SYNTAX_STRING,
        .fg_syntax_comment      = NV_FG_SYNTAX_COMMENT,
        .fg_syntax_number       = NV_FG_SYNTAX_NUMBER,
        .fg_syntax_operator     = NV_FG_SYNTAX_OPERATOR,
        .fg_syntax_function     = NV_FG_SYNTAX_FUNCTION,
        .fg_syntax_variable     = NV_FG_SYNTAX_VARIABLE,
        .fg_syntax_type         = NV_FG_SYNTAX_TYPE,
        .fg_syntax_macro        = NV_FG_SYNTAX_MACRO,
        .fg_syntax_preprocessor = NV_FG_SYNTAX_PREPROCESSOR,
        .fg_syntax_constant     = NV_FG_SYNTAX_CONSTANT,
        .fg_syntax_builtins     = NV_FG_SYNTAX_BUILTINS,
        .fg_syntax_attribute    = NV_FG_SYNTAX_ATTRIBUTE,
        .fg_syntax_error        = NV_FG_SYNTAX_ERROR,
    };

    return NV_OK;
}

void nv_log(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    struct nv_context logger = nv_get_context(nv_editor->logger);
    if (!logger.buffer || !logger.buffer->buffer) {
        va_end(ap);
        return;
    }

    size_t cur = logger.buffer->append_cursor, cap = logger.buffer->chunk_size;
    char* buf = logger.buffer->buffer;
    if (cur >= cap || cap == 0) {
        va_end(ap);
        return;
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double time_taken;
    time_taken = (now.tv_sec - nv_editor->start.tv_sec) * 1e9;
    time_taken = (time_taken + (now.tv_nsec - nv_editor->start.tv_nsec)) * 1e-9;
    int ts_written = snprintf(buf + cur, cap - cur, "[%.3f] ", time_taken);

    if (ts_written <= 0) {
        va_end(ap);
        return;
    }

    if ((size_t)ts_written >= cap - cur) {
        va_end(ap);
        return;
    }

    cur += (size_t)ts_written;

    va_list ap2;
    va_copy(ap2, ap);
    int msg_len = vsnprintf(buf + cur, cap - cur, fmt, ap2);
    va_end(ap2);

    if (msg_len < 0) {
        va_end(ap);
        return;
    }

    if ((size_t)msg_len >= cap - cur) {
        logger.buffer->append_cursor = 0;
        va_end(ap);
        return;
    }

    for (size_t i = 0, end = cur + (size_t)msg_len; cur + i < end; ++i) {
        if (buf[cur + i] == '\n') {
            logger.buffer->line_count++;
        }
    }

    cur += (size_t)msg_len;
    buf[cur] = '\0';
    logger.buffer->append_cursor = cur;
    cvector_set_size(logger.buffer->buffer, logger.buffer->append_cursor);

    va_end(ap);
}

void nv_fatal(const char* operation)
{
    nv_editor->running = false;
    nv_log("nv: %s: %s, %d", operation ? operation : "unknown operation", nv_strerror(nv_editor->status), nv_editor->status);
    exit(nv_editor->status);
}

static void nv_redraw_all()
{
    if (nv_editor->config.show_headless) {
        return;
    }

    nv_calculate_statline();
    nv_draw_background(); // clear
    nv_draw_windows(nv_editor->window, (struct nv_window_area) { 0, 0, nv_editor->width, nv_editor->height });
    if (nv_editor->logger->leaf.view->visible) {
        nv_draw_windows(nv_editor->logger,
            (struct nv_window_area) {
                .x=0.5 * nv_editor->width * 0.2,
                .y=0.5 * nv_editor->height * 0.2,
                .w=nv_editor->width * 0.8,
                .h=nv_editor->height * 0.8
        });
    }
    nv_draw_cursor();
    nv_draw();
}

void nv_resize_for_layout(int width, int height)
{
    nv_editor->width = width;
    nv_editor->height = height;
}

static void nv_on_poller_closed(uv_handle_t* handle)
{
    free(handle);
}

static void nv_on_tty_closed(uv_handle_t* handle)
{
    free(handle);
}

static void nv_close_pollers()
{
    for (int i = 0; i < NV_POLLER_COUNT; i++) {
        if (!nv_editor->pollers[i]) {
            continue;
        }
        uv_poll_stop(nv_editor->pollers[i]);
        uv_close((uv_handle_t*)nv_editor->pollers[i], nv_on_poller_closed);
        nv_editor->pollers[i] = NULL;
    }
}

static void nv_on_tty(uv_poll_t* handle, int status, int events)
{
    if ((events & UV_READABLE) != true) {
        return;
    }

    struct tb_event ev;
    tb_peek_event(&ev, 0); // consume
    nv_get_input(&ev);
    nv_redraw_all();
}

static void nv_on_nng_recv(uv_poll_t* handle, int status, int events)
{
    if ((events & UV_READABLE) != true) {
        return;
    }

    int fd;
    uv_fileno((uv_handle_t*)handle, &fd);
    if (fd < 0) {
        return;
    }

    char buffer[1024];
    ssize_t nread = recv(fd, buffer, 1024, 0);
    if (nread == 0 || nread < 1) {
        uv_poll_stop(handle);
        return;
    }

    nv_log("nvrpc received on nng recv socket\n");
    nv_editor->nvrpc->nvrpc_execute_request(buffer, (size_t)nread);
}

static void nv_on_nng_send(uv_poll_t* handle, int status, int events)
{
    if ((events & UV_WRITABLE) != true) {
        return;
    }
}

static void nv_on_tty_stream(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    (void)stream;
    (void)nread;
    (void)buf;
    nv_on_tty(NULL, 0, UV_READABLE);

    if (!nv_editor->running) {
        nv_shutdown();
    }
}

static void nv_tty_stream_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
    (void)handle;
    (void)suggested_size;
    (void)buf;
}

static void nv_register_pollers(uv_loop_t* loop, struct nv_poller_fd fds[], size_t nfds)
{
    int rv;
    uv_poll_t** poller;
    for (int i = 0; i < (int)nfds; i++) {
        if (fds[i].poller_index >= NV_POLLER_COUNT || fds[i].fd < 0) {
            continue;
        }

        poller = &nv_editor->pollers[fds[i].poller_index];
        *poller = (uv_poll_t*)malloc(sizeof(uv_poll_t));
        if (!*poller) {
            nv_editor->status = NV_ERR_MEM;
            break;
        }

        if ((rv = uv_poll_init(loop, *poller, fds[i].fd)) != 0) {
            tb_shutdown();
            printf("failed to register poller: %s\n", uv_strerror(rv));
            exit(NV_ERR);
        }
        if ((rv = uv_poll_start(*poller, UV_READABLE, fds[i].cb)) != 0) {
            tb_shutdown();
            printf("failed to start poller: %s\n", uv_strerror(rv));
            exit(NV_ERR);
        }
    }
}

static void nv_shutdown()
{
    nv_close_pollers();

    if (nv_editor->tty) {
        uv_read_stop((uv_stream_t*)nv_editor->tty);
        uv_close((uv_handle_t*)nv_editor->tty, nv_on_tty_closed);
        nv_editor->tty = NULL;
    }
}

void nv_main()
{
    if (nv_editor->running) {
        return;
    }

    tb_set_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);
#ifdef TB_OUTPUT_TRUECOLOR
    tb_set_output_mode(TB_OUTPUT_TRUECOLOR);
#else
    tb_set_output_mode(TB_OUTPUT_NORMAL);
#endif

    nv_editor->running = true;
    nv_redraw_all();

    uv_loop_t* loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
    if (!loop) {
        nv_editor->status = NV_ERR_MEM;
        exit(nv_editor->status);
    }
    uv_loop_init(loop);

    int tb_tty_fd, tb_resize_fd;
    tb_get_fds(&tb_tty_fd, &tb_resize_fd);
    // register termbox pollers
    nv_editor->tty = (uv_tty_t*)malloc(sizeof(uv_tty_t));
    // tb_tty_fd is an fd for /dev/tty which cannot be kqueue'd on macos,
    // do not register that as a traditional poller
    if (uv_tty_init(loop, nv_editor->tty, STDIN_FILENO, UV_READABLE) != 0) {
        nv_editor->status = NV_ERR;
        exit(nv_editor->status);
    }
    (void)uv_tty_set_mode(nv_editor->tty, UV_TTY_MODE_RAW);
    (void)uv_read_start((uv_stream_t*)nv_editor->tty, nv_tty_stream_alloc, nv_on_tty_stream);

    nv_register_pollers(loop, (struct nv_poller_fd[]) {
        {
            .fd = tb_resize_fd,
            .poller_index = NV_POLLER_INDEX_RESIZE,
            .cb = nv_on_tty,
        }
    }, 1);

    // register nng pollers if rpc api is on
    if (nv_editor->nvrpc) {
        nv_register_pollers(loop, (struct nv_poller_fd[]) {
            {
                .fd = nv_editor->nvrpc->nng_recv_fd,
                .poller_index = NV_POLLER_INDEX_NNG_RECV,
                .cb = nv_on_nng_recv,
            },
            {
                .fd = nv_editor->nvrpc->nng_send_fd,
                .poller_index = NV_POLLER_INDEX_NNG_SEND,
                .cb = nv_on_nng_send,
            }
        }, 2);
    }

    uv_run(loop, UV_RUN_DEFAULT);

    nv_shutdown();

    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);
}

static int nv_get_input(struct tb_event* ev)
{
    if (nv_editor->config.show_headless) {
        return NV_OK;
    }

    nv_editor->inputs[0] = ev->key;
    nv_editor->inputs[1] = 0;

    switch (ev->type) {
    case TB_EVENT_MOUSE:
        nv_handle_mouse_input(ev);
        break;

    case TB_EVENT_KEY:
        nv_handle_key_input(ev);
        break;

    case TB_EVENT_RESIZE:
        if (!nv_editor->config.show_headless) {
            nv_resize_for_layout(tb_width(), tb_height());
        }
        break;

    default:
        break;
    }

    // cursor->line = cursor->line < 1 ? 1 : cursor->line;
    // cursor->line = cursor->line > ctx.buffer->line_count ? ctx.buffer->line_count : cursor->line;
    // ctx.view->top_line_index = ctx.view->top_line_index < 1 ? 1 : ctx.view->top_line_index;
    // ctx.view->top_line_index = ctx.view->top_line_index > ctx.buffer->line_count ? ctx.buffer->line_count : ctx.view->top_line_index;

    return NV_OK;
}