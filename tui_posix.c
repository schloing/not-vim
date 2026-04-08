#include "cvector.h"
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <color.h>
#include <tui.h>
#include <error.h>

static struct {
    cvector(struct nv_hl) nv_hls;
    struct nv_tui_cell* new;
    struct nv_tui_cell* curr;
    size_t width;
    size_t height;
    struct termios termios0;
} nv_tui_state;

static volatile sig_atomic_t nv_tui_resize_pending = 0;

static int nv_tui_query_dimensions(size_t* width, size_t* height);
static int nv_tui_resize(size_t width, size_t height);
static void nv_tui_handle_resize(int sig);
static int nv_tui_process_resize(void);

size_t nv_tui_width()
{
    return nv_tui_state.width;
}

size_t nv_tui_height()
{
    return nv_tui_state.height;
}

static int nv_tui_query_dimensions(size_t* width, size_t* height)
{
    if (!width || !height) {
        return NV_ERR_NOT_INIT;
    }

    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        return NV_ERR;
    }

    *width = w.ws_col;
    *height = w.ws_row;

    return NV_OK;
}

static int nv_tui_resize(size_t width, size_t height)
{
    size_t count = width * height;
    size_t bytes = count * sizeof(struct nv_tui_cell);

    struct nv_tui_cell* curr_new = (struct nv_tui_cell*)realloc(nv_tui_state.curr, bytes);
    if (!curr_new && count) {
        return NV_ERR;
    }

    struct nv_tui_cell* new_new = (struct nv_tui_cell*)realloc(nv_tui_state.new, bytes);
    if (!new_new && count) {
        return NV_ERR;
    }

    nv_tui_state.curr = curr_new;
    nv_tui_state.new = new_new;
    nv_tui_state.width = width;
    nv_tui_state.height = height;

    if (count) {
        memset(nv_tui_state.curr, 0, bytes);
        memset(nv_tui_state.new, 0, bytes);
    }

    return NV_OK;
}

static void nv_tui_handle_resize(int sig)
{
    (void)sig;
    nv_tui_resize_pending = 1;
}

static int nv_tui_process_resize(void)
{
    size_t width;
    size_t height;

    if (nv_tui_query_dimensions(&width, &height) != NV_OK) {
        return NV_ERR;
    }

    return nv_tui_resize(width, height);
}

static bool nv_tui_cell_eq(struct nv_tui_cell a, struct nv_tui_cell b)
{
    return a.rune == b.rune && a.hl == b.hl && a.flags == b.flags;
}

static bool nv_tui_same_style(struct nv_tui_cell a, struct nv_tui_cell b)
{
    return a.hl == b.hl && a.flags == b.flags;
}

void nv_tui_present()
{
    if (nv_tui_resize_pending) {
        nv_tui_resize_pending = 0;
        (void)nv_tui_process_resize();
    }

    cvector(char) buf = NULL;
    cvector_reserve(buf, 1024);

    size_t width = nv_tui_state.width;
    size_t height = nv_tui_state.height;

    for (size_t row = 0; row < height; row++) {
        size_t start = row * width;
        struct nv_tui_cell* new_row  = &nv_tui_state.new[start];
        struct nv_tui_cell* curr_row = &nv_tui_state.curr[start];

        if (memcmp(new_row, curr_row, width * sizeof(struct nv_tui_cell)) == 0) {
            continue;
        }

        size_t col = 0;
        while (col < width) {
            if (nv_tui_cell_eq(new_row[col], curr_row[col])) {
                col++;
                continue;
            }

            size_t run_start = col;
            col++;

            while (col < width &&
                !nv_tui_cell_eq(new_row[col], curr_row[col]) &&
                nv_tui_same_style(new_row[col], new_row[run_start])) {
                col++;
            }

            size_t off = cvector_size(buf);
            cvector_reserve(buf, off + 64 + (col - run_start));
            struct nv_hl hl = nv_tui_state.nv_hls[new_row[run_start].hl];

            if (new_row[run_start].flags & NV_TUI_FLAGS_INVERT) {
                uint32_t tmp = hl.bg;
                hl.bg = hl.fg;
                hl.fg = tmp;
            }

#define NV_LINEAR_HEX_TO_RGB(c) (((c) >> 16) & 0xff), (((c) >> 8) & 0xff), ((c) & 0xff)

            int size = snprintf(
                buf + off,
                cvector_capacity(buf) - off,
                "\x1b[38;2;%d;%d;%d;48;2;%d;%d;%dm\x1b[%zu;%zuH",
                NV_LINEAR_HEX_TO_RGB(hl.fg),
                NV_LINEAR_HEX_TO_RGB(hl.bg),
                row + 1,
                run_start + 1
            );

            cvector_set_size(buf, off + size);

            for (size_t k = run_start; k < col; k++) {
                char ch = new_row[k].rune ? (char)new_row[k].rune : ' ';
                buf[cvector_size(buf)] = ch;
                cvector_set_size(buf, cvector_size(buf) + 1);
                curr_row[k] = new_row[k];
            }
        }
    }

    memset(nv_tui_state.new, 0, width * height * sizeof(struct nv_tui_cell));

    if (cvector_size(buf)) {
        (void)write(STDOUT_FILENO, buf, cvector_size(buf));
    }

    cvector_free(buf);
}

void nv_tui_invert_cell(int x, int y)
{
    if (x < 0 || y < 0) {
        return;
    }
    if ((size_t)x >= nv_tui_state.width || (size_t)y >= nv_tui_state.height) {
        return;
    }

    size_t linear = (size_t)y * nv_tui_state.width + (size_t)x;
    nv_tui_state.new[linear].flags |= NV_TUI_FLAGS_INVERT;
}

void nv_tui_set_cell(int x, int y, uint32_t ch, hl_index hl)
{
    if (x < 0 || y < 0) {
        return;
    }
    if ((size_t)x >= nv_tui_state.width || (size_t)y >= nv_tui_state.height) {
        return;
    }

    size_t linear = (size_t)y * nv_tui_state.width + (size_t)x;
    nv_tui_state.new[linear].rune = ch;
    nv_tui_state.new[linear].hl = hl;
}

void nv_tui_printf(int x, int y, hl_index hl, const char* fmt, ...)
{
    char buf[128];

    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    if (len < 0) {
        return;
    }
    if ((size_t)len >= sizeof(buf)) {
        len = sizeof(buf) - 1;
    }

    for (int i = 0; i < len; i++) {
        nv_tui_set_cell(x + i, y, buf[i], hl);
    }
}

int nv_tui_init()
{
    int rv = NV_OK;

    if ((rv = nv_tui_process_resize()) != NV_OK) {
        return rv;
    }

    signal(SIGWINCH, nv_tui_handle_resize);

    cvector_reserve(nv_tui_state.nv_hls, 32);

    nv_tui_state.nv_hls[NV_TUI_HL_BLACK_ON_WHITE] = (struct nv_hl){
        .fg = NV_BLACK,
        .bg = NV_WHITE,
        .attr = 0,
    };
    nv_tui_state.nv_hls[NV_TUI_HL_WHITE_ON_BLACK] = (struct nv_hl){
        .fg = NV_WHITE,
        .bg = NV_BLACK,
        .attr = 0,
    };

    struct termios raw;
    (void)tcgetattr(STDIN_FILENO, &raw);
    nv_tui_state.termios0 = raw;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    (void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    (void)write(STDOUT_FILENO, "\x1b[?25l", 6);

    return rv;
}

static void nv_tui_deinit()
{
    (void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &nv_tui_state.termios0);
    (void)write(STDOUT_FILENO, "\x1b[?25h", 6);
    (void)write(STDOUT_FILENO, "\x1b[2J", 4);
    (void)write(STDOUT_FILENO, "\x1b[H", 3);
}

void nv_tui_free()
{
    nv_tui_deinit();

    free(nv_tui_state.curr);
    free(nv_tui_state.new);
    cvector_free(nv_tui_state.nv_hls);

    nv_tui_state.curr = NULL;
    nv_tui_state.new = NULL;
    nv_tui_state.width = 0;
    nv_tui_state.height = 0;
}

void nv_tui_clear()
{
    size_t count = nv_tui_state.width * nv_tui_state.height;

    if (count) {
        memset(nv_tui_state.curr, 0,
               count * sizeof(struct nv_tui_cell));
        memset(nv_tui_state.new, 0,
               count * sizeof(struct nv_tui_cell));
    }

    (void)write(STDOUT_FILENO, "\x1b[2J", 4);
    (void)write(STDOUT_FILENO, "\x1b[H", 3);
}