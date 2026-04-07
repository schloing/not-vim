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
    struct nv_tui_cell* curr = realloc(
        nv_tui_state.curr,
        count * sizeof(struct nv_tui_cell)
    );
    if (!curr && count != 0) {
        return NV_ERR;
    }

    struct nv_tui_cell* new = realloc(
        nv_tui_state.new,
        count * sizeof(struct nv_tui_cell)
    );
    if (!new && count != 0) {
        return NV_ERR;
    }

    nv_tui_state.curr = curr;
    nv_tui_state.new = new;
    nv_tui_state.width = width;
    nv_tui_state.height = height;

    if (count != 0) {
        memset(nv_tui_state.curr, 0, count * sizeof(struct nv_tui_cell));
        memset(nv_tui_state.new, 0, count * sizeof(struct nv_tui_cell));
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
    return a.rune == b.rune && a.hl == b.hl;
}

void nv_tui_present()
{
    if (nv_tui_resize_pending) {
        nv_tui_resize_pending = 0;
        (void)nv_tui_process_resize();
    }

    cvector(char) buf = NULL;
    cvector_reserve(buf, 1024);

    int width = nv_tui_state.width;
    int height = nv_tui_state.height;

    for (int row = 0; row < height; row++) {
        int start = row * width;
        struct nv_tui_cell* new_row = &nv_tui_state.new[start];
        struct nv_tui_cell* curr_row = &nv_tui_state.curr[start];

        if (memcmp(new_row, curr_row, width * sizeof(struct nv_tui_cell)) == 0) {
            continue;
        }

        int col = 0;
        while (col < width) {
            if (nv_tui_cell_eq(new_row[col], curr_row[col])) {
                col++;
                continue;
            }

            int run_start = col;

            while (col < width && !nv_tui_cell_eq(new_row[col], curr_row[col])) {
                col++;
            }

            int off = cvector_size(buf);

            cvector_reserve(buf, off + 64 + (col - run_start));

            int size = snprintf(
                buf + off,
                cvector_capacity(buf) - off,
                "\x1b[%d;%dH",
                row + 1,
                run_start + 1
            );

            cvector_set_size(buf, off + size);

            for (int k = run_start; k < col; k++) {
                buf[cvector_size(buf)] = new_row[k].rune;
                cvector_set_size(buf, cvector_size(buf) + 1);

                curr_row[k] = new_row[k];
            }
        }
    }

    write(STDOUT_FILENO, buf, cvector_size(buf));
    cvector_free(buf);
}

void nv_tui_set_cell(int x, int y, uint32_t ch, uint32_t fg, uint32_t bg)
{
    (void)fg;
    (void)bg;

    if (x < 0 || y < 0) {
        return;
    }
    if ((size_t)x >= nv_tui_state.width || (size_t)y >= nv_tui_state.height) {
        return;
    }

    int linear = y * nv_tui_state.width + x;
    nv_tui_state.new[linear].rune = ch;
    // TODO: set hl based on fg/bg
}

void nv_tui_printf(int x, int y, uint32_t fg, uint32_t bg, const char* fmt, ...)
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
        nv_tui_set_cell(x + i, y, buf[i], fg, bg);
    }
}

int nv_tui_init()
{
    int rv = NV_OK;

    if ((rv = nv_tui_process_resize()) != NV_OK) {
        return rv;
    }

    signal(SIGWINCH, nv_tui_handle_resize);

    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    nv_tui_state.termios0 = raw;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    (void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    return rv;
}

static void nv_tui_deinit()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &nv_tui_state.termios0);
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void nv_tui_free()
{
    nv_tui_deinit();

    free(nv_tui_state.curr);
    free(nv_tui_state.new);

    nv_tui_state.curr = NULL;
    nv_tui_state.new = NULL;
    nv_tui_state.width = 0;
    nv_tui_state.height = 0;
}

void nv_tui_clear()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
}