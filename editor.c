#include "termbox2.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "buffer.h"
#include "color.h"
#include "cursor.h"
#include "cursorhelp.h"
#include "cvector.h"
#include "editor.h"
#include "error.h"
#include "window.h"

extern int tb_clear_region(int, int);
static int nv_get_input(struct nv_editor* editor, struct tb_event* ev);
static struct nv_buff* nv_get_active_buffer(struct nv_editor* editor);
static int count_recur(int n);
static void nv_draw_buffer_within_window(struct nv_window* window, struct nv_buff* buffer);
static void nv_set_mode(struct nv_editor* editor, nv_mode mode);
static void nv_draw_cursor(struct nv_editor* editor);
static void nv_redraw_all(struct nv_editor* editor);
static void nv_draw_background(struct nv_editor* editor);
static int nv_draw_windows(struct nv_window* root);
static int nv_draw_buffer(struct nv_window* window);
static int nv_draw_status(struct nv_editor* editor);

int nv_editor_init(struct nv_editor* editor)
{
    if (!editor) {
        return NV_ERR_NOT_INIT;
    }

    editor->mode = (nv_mode)NV_MODE_NAVIGATE;

    editor->nv_conf = (struct nv_conf) {
        .tab_width = NV_TAB_WIDTH,
        .expand_tab = NV_TAB_WIDTH,
        .auto_indent = NV_AUTO_INDENT,
        .line_numbers = NV_LINE_NUMBERS,
        .show_relative = NV_SHOW_RELATIVE,
        .command_delay = NV_COMMAND_DELAY,
        .show_headless = NV_HEADLESS,
    };

    return NV_OK;
}

static void nv_set_mode(struct nv_editor* editor, nv_mode mode)
{
    editor->mode = mode;
    nv_draw_status(editor);
}

static void nv_draw_cursor(struct nv_editor* editor)
{
    struct nv_buff* buffer = nv_get_active_buffer(editor);
    struct cursor c = buffer->cursors[0];
    int row = line(buffer, c.line)->length;
    int effective_row = (c.x > row ? row : c.x) < 0 ? 0 : (c.x > row ? row : c.x); // FIXME
    tb_set_cell(buffer->linecol_size + effective_row + 1, c.y, ' ', TB_256_BLACK, TB_256_WHITE);
    tb_present();
}

static void nv_redraw_all(struct nv_editor* editor)
{
    if (editor->nv_conf.show_headless) {
        return;
    }

    nv_draw_background(editor); // clear
    nv_draw_windows(editor->window);
    nv_draw_status(editor);
    tb_present();
}

void nv_mainloop(struct nv_editor* editor)
{
    tb_set_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);
    tb_set_output_mode(TB_OUTPUT_256);

    editor->running = true;
    nv_redraw_all(editor);

    struct tb_event ev;

    while (editor->running) {
        editor->status = tb_poll_event(&ev);

        switch (ev.type) {
        case TB_EVENT_MOUSE:
        case TB_EVENT_KEY:
            nv_draw_windows(editor->window);
            nv_get_input(editor, &ev);
            editor->running = false;

            break;

        case TB_EVENT_RESIZE:
            if (editor->nv_conf.show_headless) {
                break;
            }

            editor->height = tb_height();
            editor->width = tb_width();

            nv_redraw_all(editor);

            break;

        default:
            break;
        }
    }
}

static void nv_draw_background(struct nv_editor* editor)
{
    tb_set_clear_attrs(TB_256_WHITE, TB_256_BLACK);
    tb_clear();
}

// FIXME:
static int nv_get_input(struct nv_editor* editor, struct tb_event* ev)
{
    if (editor->nv_conf.show_headless) {
        return NV_OK;
    }

    struct nv_buff* buffer = nv_get_active_buffer(editor);
    if (!buffer || !buffer->cursors) {
        return NV_ERR_NOT_INIT;
    }

    struct cursor* cursor = &buffer->cursors[0];

    editor->inputs[0] = ev->key;
    editor->inputs[1] = 0;

    if (ev->type == TB_EVENT_MOUSE) {
        switch (ev->key) {
        case TB_KEY_MOUSE_WHEEL_UP:
            if (buffer->top_line > 0) {
                buffer->top_line--;
            }
            // nv_cursor_move_up(buffer, cursor, 1);
            break;

        case TB_KEY_MOUSE_WHEEL_DOWN:
            if (buffer->top_line < buffer->line_count) {
                buffer->top_line++;
            }
            // nv_cursor_move_down(buffer, cursor, 1);
            break;
        }
    } else {
        if (editor->mode == NV_MODE_INSERT) {
            if (isprint(ev->ch)) {
                nv_cursor_insert_ch(buffer, cursor, ev->ch);
            } else {
                switch (ev->key) {
                case TB_KEY_ESC:
                    nv_set_mode(editor, NV_MODE_NAVIGATE);
                }
            }
        } else {
            if (ev->key == TB_KEY_ESC) {
                editor->running = false;
            }

            switch (ev->ch) {
            case 'i':
                nv_set_mode(editor, NV_MODE_INSERT);
                break;

            case 'j':
                nv_cursor_move_down(buffer, cursor, 1);
                break;

            case 'k':
                nv_cursor_move_up(buffer, cursor, 1);
                break;

            case 'h':
                nv_cursor_move_left(buffer, cursor, 1);
                break;

            case 'l':
                nv_cursor_move_right(buffer, cursor, 1);
                break;
            }
        }
    }

    //  _nv_draw_buffer(editor);
    nv_draw_cursor(editor);
    return NV_OK;
}

// FIXME
static struct nv_buff* nv_get_active_buffer(struct nv_editor* editor)
{
    //  struct nv_buff* buffer = (struct nv_buff*)&editor->windows[editor->peek];
    //  editor->current = buffer;
    //  return buffer;
    return NULL;
}

// calculate width of number
static int count_recur(int n)
{
    if (n < 0) {
        return count_recur((n == INT_MIN) ? INT_MAX : -n);
    }
    if (n < 10) {
        return 1;
    }
    return 1 + count_recur(n / 10);
}

static int nv_draw_windows(struct nv_window* root)
{
    if (!root) {
        return NV_ERR_NOT_INIT;
    }

    if (root->has_children) {
        nv_draw_windows(root->left);
        nv_draw_windows(root->right);
        return NV_OK;
    }

    if (root->show) {
        nv_draw_buffer(root);
    }

    return NV_OK;
}

static void nv_draw_buffer_within_window(struct nv_window* window, struct nv_buff* buffer)
{
    char* lbuf = calloc(window->wd.w, sizeof(char));
    
    if (!lbuf) {
        // FIXME
        //  return ERR_MEM;
        return;
    }
   
    size_t line_no = 0;
    size_t max_width = window->wd.w - (buffer->linecol_size + 1);

    for (int row = window->wd.y; row < window->wd.y + window->wd.h; row++) {
        if (line_no > buffer->line_count) {
            continue;
        }

        struct nv_buff_line* line = &buffer->lines[line_no++];
        size_t copy_size = (line->length > max_width) ? max_width : line->length;
        memcpy(lbuf, &buffer->buffer[line->begin], copy_size);
        lbuf[copy_size] = 0;
        tb_printf(window->wd.x, row, TB_256_WHITE, TB_256_BLACK, "%*d %s", buffer->linecol_size, line_no, lbuf);

        if (line->length > max_width) {
            size_t num_wraps = line->length / window->wd.w;

            for (size_t i = 1; i <= num_wraps; i++) {
                size_t offset = i * max_width;

                if (offset >= line->length) {
                    break;
                }

                size_t wrap_size = (line->length - offset > max_width) ? max_width : line->length - offset;
                memcpy(lbuf, &buffer->buffer[line->begin + offset], wrap_size);
                lbuf[wrap_size] = '\0';

                tb_printf(window->wd.x, ++row, TB_256_WHITE, TB_256_BLACK, "%*c %s", buffer->linecol_size, ' ', lbuf);
            }
        }
    }

    free(lbuf);
}

static int nv_draw_buffer(struct nv_window* window)
{
    if (!window || !window->show || !window->buffer) {
        return NV_ERR_NOT_INIT;
    }

    struct nv_buff* buffer = window->buffer;

    switch (buffer->type) {
    case NV_BUFFTYPE_PLAINTEXT:
    case NV_BUFFTYPE_SOURCE:
        // int top = buffer->cursors[0].line - buffer->cursors[0].y;

        if (!buffer->loaded) {
            nv_load_file_buffer(buffer, &buffer->line_count);
            buffer->linecol_size = count_recur(buffer->line_count);
            buffer->loaded = true;
        }

        nv_draw_buffer_within_window(window, buffer);
        break;

    case NV_BUFFTYPE_BROWSER:
        tb_print(0, 0, TB_256_WHITE, TB_256_BLACK, "netrw");
        break;

    default:
        fprintf(stderr, "unsupported bufftype %d\n", buffer->type);
        break;
    }

    return NV_OK;
}

// extern char* nv_mode_str
char* nv_mode_str[NV_MODE_INSERTS + 1] = {
    "NAV",
    "INS",
    "HIGH",
    "INS*",
};

static int nv_draw_status(struct nv_editor* editor)
{
    //  struct nv_buff* buffer = __nv_get_active_buffer(editor);
    //  char* prompt;
    //  if (asprintf(&prompt, "%s[%zu] %s", nv_mode_str[editor->mode], buffer->id, buffer->path) ==
    //  -1) return; tb_printf(0, editor->height - 1, TB_256_BLACK, TB_256_WHITE, "%-*.*s",
    //  editor->width, editor->width, prompt); free(prompt);
    return NV_OK;
}
