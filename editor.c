#include <ctype.h>
#include <dirent.h>
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
#include "termbox2.h"
#include "window.h"

static int nv_get_input(struct tb_event* ev);
static int count_no_digits(int n);
static struct nv_window* nv_get_active_window();
static int nv_draw_buffer_within_window(struct nv_window* window);
static void nv_set_mode(nv_mode mode);
static void nv_draw_cursor();
static void nv_redraw_all();
static void nv_draw_background();
static int nv_draw_windows(struct nv_window* root);
static int nv_draw_buffer(struct nv_window* window);
static int nv_draw_status();

_Thread_local struct nv_editor* nv_editor = NULL; // extern in editor.h 

// extern in editor.h
char* nv_mode_str[NV_MODE_INSERTS + 1] = {
    "NAV",
    "INS",
    "HIGH",
    "INS*",
};

// extern in buffer.h
char* nv_bufftype_str[NV_BUFFTYPE_LOG + 1] = {
    "stdin",
    "stdout",
    "browser",
    "network",
    "source",
    "plaintext",
    "log",
};

// extern in buffer.h
char* nv_bufffmt_str[NV_BUFFTYPE_PLAINTEXT + 1] = {
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

static void nv_set_mode(nv_mode mode)
{
    nv_editor->mode = mode;
    nv_draw_status(nv_editor);
}

static void nv_draw_cursor()
{
    struct nv_context ctx = nv_get_context(nv_get_active_window());

    struct cursor c;
    int line_length = 0;
    int effective_row = 0;
    
    for (int cindex = 0; cindex <= cvector_size(ctx.buffer->cursors); cindex++) {
        c = ctx.buffer->cursors[cindex];
        line_length = line(ctx.view, c.line)->length;

        effective_row =
            ctx.window->wd.x + // window position
            ctx.view->gutter_digit_width + 1 + // space taken by line numbers
            (c.x > line_length ? line_length : c.x); // cap the cursor to the end of the line

        tb_set_cell(effective_row, c.y, ' ', NV_BLACK, NV_WHITE);
    }

    tb_present();
}

void nv_log(const char* fmt, ...)
{
    // FIXME
    va_list ap;
    va_start(ap, fmt);
    // nv_buffer_printf(editor->buffer, fmt, ap);
    va_end(ap);
}

void nv_fatal(const char* operation)
{
    // FIXME
    // dont work like a fatal error function should
    nv_editor->running = false;
    nv_log("nv: %s: %s, %d", operation ? operation : "unknown operation", nv_strerror(nv_editor->status), nv_editor->status);
}

static void nv_redraw_all()
{
    if (nv_editor->config.show_headless) {
        return;
    }

    nv_draw_background(nv_editor); // clear
    nv_draw_windows(nv_editor->window); // TODO: add log buffer override
    nv_draw_status(nv_editor);
    tb_present();
}

void nv_resize_for_layout(size_t width, size_t height) {
    nv_editor->width = width;
    nv_editor->height = nv_editor->statline ? height - nv_editor->statline->height : height;
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
    nv_redraw_all(nv_editor);
    nv_draw_cursor();

    struct tb_event ev;

    while (nv_editor->running) {
        nv_editor->status = tb_poll_event(&ev);

        switch (ev.type) {
        case TB_EVENT_MOUSE:
            nv_editor->running = false; // FIXME

            break;

        case TB_EVENT_KEY:
            nv_get_input(&ev);
            nv_draw_windows(nv_editor->window);

            break;

        case TB_EVENT_RESIZE:
            if (nv_editor->config.show_headless) {
                break;
            }

            nv_resize_for_layout(tb_width(), tb_height());
            NV_WD_SET_SIZE(nv_editor->window->wd, nv_editor->width, nv_editor->height);
            nv_redistribute(nv_editor->window);
            nv_redraw_all(nv_editor);

            break;

        default:
            break;
        }
    }
}

static void nv_draw_background()
{
    tb_set_clear_attrs(nv_editor->config.fg_main, nv_editor->config.bg_main);
    tb_clear();
}

struct nv_context nv_get_context(struct nv_window* window) {
    struct nv_context ctx = {
        .window = window,
        .view = window->view,
    };
    
    ctx.buffer = ctx.view ? window->view->buffer : NULL;

    return ctx;
}

static int nv_get_input(struct tb_event* ev)
{
    if (nv_editor->config.show_headless) {
        return NV_OK;
    }

    struct nv_context ctx = nv_get_context(nv_get_active_window());

    if (!ctx.buffer || !ctx.buffer->cursors) {
        return NV_ERR_NOT_INIT;
    }

    struct cursor* cursor = &ctx.buffer->cursors[0];

    nv_editor->inputs[0] = ev->key;
    nv_editor->inputs[1] = 0;

    if (ev->type == TB_EVENT_MOUSE) {
        switch (ev->key) {
        case TB_KEY_MOUSE_WHEEL_UP:
            if (ctx.view->top_line_index > 0) {
                ctx.view->top_line_index--;
            }
            nv_cursor_move_up(ctx.view, cursor, 1);
            break;

        case TB_KEY_MOUSE_WHEEL_DOWN:
            if (ctx.view->top_line_index < ctx.view->line_count) {
                ctx.view->top_line_index++;
            }
            nv_cursor_move_down(ctx.view, cursor, 1);
            break;
        }
    } else {
        if (nv_editor->mode == NV_MODE_INSERT) {
            if (isprint(ev->ch)) {
                nv_cursor_insert_ch(ctx.view, cursor, ev->ch);
                nv_redraw_all();
            } else {
                switch (ev->key) {
                case TB_KEY_ESC:
                    nv_set_mode(NV_MODE_NAVIGATE);
                }
            }
        } else {
            if (ev->key == TB_KEY_ESC) {
                nv_editor->running = false;
            }

            switch (ev->ch) {
            case 'i':
                nv_set_mode(NV_MODE_INSERT);
                break;

            case 'j':
                nv_cursor_move_down(ctx.view, cursor, 1);
                break;

            case 'k':
                nv_cursor_move_up(ctx.view, cursor, 1);
                break;

            case 'h':
                nv_cursor_move_left(ctx.view, cursor, 1);
                break;

            case 'l':
                nv_cursor_move_right(ctx.view, cursor, 1);
                break;
            }
        }
    }

    nv_draw_buffer(nv_editor->window);
    nv_draw_cursor();

    return NV_OK;
}

static struct nv_window* nv_get_active_window()
{
    if (!nv_editor->focus) {
        return NULL;
    }

    return nv_editor->focus;
}

// calculate the # of digits in n
static int count_no_digits(int n)
{
    if (n < 0) {
        return count_no_digits((n == INT_MIN) ? INT_MAX : -n);
    }

    if (n < 10) {
        return 1;
    }
    
    return 1 + count_no_digits(n / 10);
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

// wrappers

#define NV_PRINTF(x, y, fg, fmt, ...) \
    tb_printf(x, y, fg, nv_editor->config.bg_main, fmt, ##__VA_ARGS__)

static inline void nv_printf_buffer(struct nv_context* ctx, int row, int line_no, const char* lbuf)
{
    NV_PRINTF(ctx->window->wd.x, row, NV_GRAY, "%*d", ctx->view->gutter_digit_width, line_no);
    NV_PRINTF(ctx->window->wd.x + ctx->view->gutter_digit_width + 1, row, NV_WHITE, "%s", lbuf);
}

static void nv_rebuild_visual_rows(struct nv_context* ctx)
{
    struct nv_buff_line* current_line = NULL;
    int max_width = window->wd.w - (ctx.view->gutter_digit_width + 1);
    int buff_line_no = ctx.view->top_line_index;

    for (int row = window->wd.y; row < window->wd.y + window->wd.h; row++, buff_line_no++) {
        current_line = line(ctx.view, buff_line_no);

        if (!current_line) {
            nv_editor->status = NV_ERR;
            return;
        }

        struct nv_visual_row vr = {
            .line_index = buff_line_no,
            .wrap_index = 0,
            .offset = 0,
        };

        cvector_push_back(ctx.view->visual_rows, vr);
        ctx.view->map[buff_line_no - ctx.view->top_line_index] = cvector_size(ctx.view->visual_rows) - 1;

        if (current_line->length > max_width) {
            size_t num_wraps = current_line->length / max_width;

            for (size_t i = 1; i <= num_wraps; i++) {
                size_t offset = i * max_width;

                if (offset >= line_length) {
                    break;
                }

                vr.wrap_index = i;
                vr.offset = offset;
                cvector_push_back(ctx.view->visual_rows, vr);
            }
        }
    }
}

static int nv_draw_buffer_within_window(struct nv_window* window)
{
    char* lbuf = calloc(window->wd.w, sizeof(char));
    
    if (!lbuf) {
        return NV_ERR_MEM;
    }

    struct nv_context ctx = nv_get_context(window);

    if (ctx.view->line_count <= 0) {
        free(lbuf);
        return NV_OK;
    }

    if (cvector_size(visual_rows) <= 0) {
        nv_rebuild_visual_rows(&ctx);
    }
   
    struct nv_buff_line* current_line = NULL;
    size_t line_length = 0;
    size_t line_no = 0;
    size_t copy_size = 0;

    for (int row = window->wd.y; row < window->wd.y + window->wd.h; row++) {
        current_line = line(ctx.view, line_no++);

        if (!current_line) {
            break;
        }

        line_length = current_line->end - current_line->begin;

        if (line_length == 0) {
            nv_printf_buffer(&ctx, row, line_no, " ");
        }

        copy_size = (line_length > max_width) ? max_width : line_length;

        // convert invalid characters into their printable variants
        int r = 0;
        for (int i = 0; i < copy_size; i++) {
            char chr = ctx.buffer->buffer[current_line->begin + i];

            if (chr == '\t') {
                // tabs need to be expanded into spaces to nearest tab width
                size_t space_count = nv_editor->config.tab_width - (r % nv_editor->config.tab_width);
                memset(lbuf + r, ' ', space_count);
                r += space_count;
            } else {
                lbuf[r++] = chr;
            }
        }

        lbuf[r] = '\0';
        nv_printf_buffer(&ctx, row, line_no, lbuf);

        // TODO:
        // handle the tab expansion here as well
        if (line_length > max_width) {
            size_t num_wraps = line_length / max_width;

            for (size_t i = 1; i <= num_wraps; i++) {
                size_t offset = i * max_width;
                
                if (offset >= line_length) {
                    break;
                }
                
                row++;
                size_t wrap_size = (line_length - offset > max_width) ? max_width : line_length - offset;
                memcpy(lbuf, &ctx.buffer->buffer[current_line->begin + offset], wrap_size);
                lbuf[wrap_size] = '\0';
                nv_printf_buffer(&ctx, row, line_no, lbuf);
            }
        }
    }

    free(lbuf);
    return NV_OK;
}

int netrw_filename_sort(const void* a, const void* b)
{
    return strcmp(*(const char**)a, *(const char**)b);
}

static int nv_draw_buffer(struct nv_window* window)
{
    if (!window || !window->show) {
        return NV_ERR_NOT_INIT;
    }

    struct nv_context ctx = nv_get_context(window);

    if (!ctx.buffer) {
        return NV_ERR_NOT_INIT;
    }

    switch (ctx.buffer->type) {
    case NV_BUFFTYPE_PLAINTEXT:
    case NV_BUFFTYPE_SOURCE:
        // int top = buffer->cursors[0].line - buffer->cursors[0].y;

        if (!ctx.buffer->loaded) {
            nv_load_file_buffer(ctx.buffer, &ctx.view->line_count);
            ctx.view->gutter_digit_width = count_no_digits(ctx.view->line_count);
            ctx.buffer->loaded = true;
        }

        if (nv_draw_buffer_within_window(window) != NV_OK) {
            nv_fatal("failed to draw buffer");
        }

        break;

    case NV_BUFFTYPE_BROWSER:
        // TODO: implement this as a plugin
        break;

    default:
        fprintf(stderr, "unsupported bufftype %d\n", ctx.buffer->type);
        break;
    }

    return NV_OK;
}

static int nv_draw_status()
{
    struct nv_context ctx = nv_get_context(nv_get_active_window());

    if (!ctx.buffer)
        return NV_ERR_NOT_INIT;

    if (asprintf(&nv_editor->statline->format, "%s (%s, %s) --%s--", ctx.buffer->path, 
                nv_bufftype_str[ctx.buffer->type], nv_bufffmt_str[ctx.buffer->format],
                nv_mode_str[nv_editor->mode]) == -1) 
        return NV_ERR_MEM;

    for (int i = 0; i < nv_editor->statline->height; i++) {
        tb_printf(0, nv_editor->height - i, NV_BLACK, NV_WHITE,
                "%-*.*s", nv_editor->width, nv_editor->width, nv_editor->statline->format);
    }

    return NV_OK;
}
