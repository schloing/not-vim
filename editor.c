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
#include "nvtree/nvtree.h"
#include "termbox2.h"
#include "window.h"

static int nv_get_input(struct tb_event* ev);
static int count_no_digits(int n);
static struct nv_window_node* nv_get_active_window();
static int nv_draw_text_buffer(struct nv_view* view, const struct nv_window_area* area);
static void nv_set_mode(nv_mode mode);
static void nv_draw_cursor();
static void nv_redraw_all();
static void nv_draw_background_rect(int x1, int y1, int x2, int y2);
static void nv_draw_background();
static int nv_draw_windows(struct nv_window_node* root, const struct nv_window_area area);
static int nv_draw_view(struct nv_view* view, const struct nv_window_area* area);
static inline void nv_buffer_printf(struct nv_view* view, const struct nv_window_area* area, int row, int line_no, char* lbuf, size_t length);
static void nv_buffer_print_tree(nv_pool_index tree, struct nv_view* view, const struct nv_window_area* area);
static struct nv_window_node* nv_get_status_window();
static int nv_calculate_statline();

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

// wrappers

#define NV_PRINTF(x, y, fg, fmt, ...) \
    tb_printf(x, y, fg, nv_editor->config.bg_main, fmt, ##__VA_ARGS__)

// FIXME
static inline void nv_buffer_printf(struct nv_view* view, const struct nv_window_area* area, int row, int line_no, char* lbuf, size_t length)
{
    if (!lbuf || !view) {
        return;
    }

    size_t max_length = 0, gutter_offset = 0;
    max_length = length > area->w ? area->w : length;
    char* string = (char*)malloc(max_length + 1);

    for (int i = 0; i < max_length; i++) {
        string[i] = lbuf[i];
    }

    string[max_length] = '\0';

    if (view->gutter_width_cols > 0) {
        gutter_offset = view->gutter_width_cols + view->gutter_gap;
        NV_PRINTF(area->x, area->y + row, NV_GRAY, "%*d", view->gutter_width_cols, line_no);
    }

    NV_PRINTF(area->x + gutter_offset, area->y + row, NV_WHITE, "%s", string);

    free(string);
}

int nv_editor_init(struct nv_editor* editor)
{
    if (!nv_editor) {
        nv_editor = editor;
    }

    nv_editor->mode = (nv_mode)NV_MODE_NAVIGATE;
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

static void nv_set_mode(nv_mode mode)
{
    nv_editor->mode = mode;
    nv_calculate_statline();
}

static void nv_draw_cursor()
{
    struct nv_context ctx = nv_get_context(nv_get_active_window());

    struct cursor c;
    int line_length = 0;
    int effective_row = 0;

    if (!ctx.view) {
        // FIXME
        return;
    }

    for (int cindex = 0; cindex <= cvector_size(ctx.view->cursors); cindex++) {
        c = ctx.view->cursors[cindex];
        struct nv_tree_node* l = NODE_FROM_POOL(line(&ctx, c.line));

        line_length = l ? l->data.length - 1 : 0; // FIXME: only works if *local* lfcount = 1

        // FIXME
        // effective_row =
        //     ctx.view->area.x +                       // window position
        //     ctx.view->gutter_digit_width + 1 +       // space taken by line numbers
        //     (c.x > line_length ? line_length : c.x); // cap the cursor to the end of the line
    
        // tb_set_cell(effective_row, c.y, ' ', NV_BLACK, NV_WHITE);
    }
}

void nv_log(const char* fmt, ...)
{
    // FIXME
    va_list ap;
    va_start(ap, fmt);
    
//  struct nv_context ctx = {
//      .window = nv_editor->logger,
//      .view = nv_editor->logger->view,
//      .buffer = nv_editor->logger->view->buffer,
//  };
//
//  nv_buffer_printf(&ctx, fmt, ap);
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

    nv_calculate_statline();
    nv_draw_background(); // clear
    nv_draw_windows(nv_editor->window, (struct nv_window_area) { 0, 0, nv_editor->width, nv_editor->height }); // TODO: add log buffer override
    nv_draw_cursor();
    tb_present();
}

void nv_resize_for_layout(size_t width, size_t height)
{
    nv_editor->width = width;
    nv_editor->height = height;
}

#include <time.h>

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

    struct tb_event ev;

    while (nv_editor->running) {
        if (tb_poll_event(&ev) != TB_OK) {
            continue;
        }

        nv_get_input(&ev);
        nv_redraw_all();
    }
}

#define NV_DEBUG_WINDOW_BACKGROUND_COLOURS

static void nv_draw_background_rect(int x1, int y1, int x2, int y2)
{
#ifdef NV_DEBUG_WINDOW_BACKGROUND_COLOURS
    int colour = rand() % 0x1000000;
#endif

    for (int i = x1; i < x2; i++) {
        for (int j = y1; j < y2; j++) {
#ifdef NV_DEBUG_WINDOW_BACKGROUND_COLOURS
            tb_set_cell(i, j, ' ', nv_editor->config.fg_main, colour);
#else
            tb_set_cell(i, j, ' ', nv_editor->config.fg_main, nv_editor->config.bg_main);
#endif
        }
    }

    // caller needa call tb_present()
}

static void nv_draw_background()
{
    tb_set_clear_attrs(nv_editor->config.fg_main, nv_editor->config.bg_main);
    tb_clear();
}

static int nv_get_input(struct tb_event* ev)
{
    if (nv_editor->config.show_headless) {
        return NV_OK;
    }

    struct nv_context focus = nv_get_context(nv_get_active_window());

    nv_editor->inputs[0] = ev->key;
    nv_editor->inputs[1] = 0;

    switch (ev->type) {
    case TB_EVENT_MOUSE:
        switch (ev->key) {
        case TB_KEY_MOUSE_WHEEL_UP:
            if (focus.view) {
                focus.view->top_line_index--;
                focus.view->top_line_index = focus.view->top_line_index < 0 ? 0 : focus.view->top_line_index;
            }
            // nv_cursor_move_up(&ctx, cursor, 2);
            break;

        case TB_KEY_MOUSE_WHEEL_DOWN:
            if (focus.view && focus.buffer) {
                focus.view->top_line_index++;
                focus.view->top_line_index = focus.view->top_line_index > focus.buffer->line_count ? focus.buffer->line_count : focus.view->top_line_index;
            }
            // nv_cursor_move_down(&ctx, cursor, 2);
            break;
        }
        break;

    case TB_EVENT_KEY:
        if (nv_editor->mode == NV_MODE_INSERT) {
            if (isprint(ev->ch)) {
                // nv_cursor_insert_ch(&ctx, cursor, ev->ch);
            } else if (ev->key == TB_KEY_ESC) {
                nv_set_mode(NV_MODE_NAVIGATE);
            }
        }
        else {
            if (ev->key == TB_KEY_ESC) {
                nv_editor->running = false;
            } else {
                switch (ev->ch) {
                case 'i': nv_set_mode(NV_MODE_INSERT); break;
                // case 'j': nv_cursor_move_down(&ctx, cursor, 1); break;
                // case 'k': nv_cursor_move_up(&ctx, cursor, 1); break;
                // case 'h': nv_cursor_move_x(&ctx, cursor, -1); break;
                // case 'l': nv_cursor_move_x(&ctx, cursor, 1); break;
                }
            }
        }
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
static struct nv_window_node* nv_get_active_window()
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

static int nv_draw_windows(struct nv_window_node* root, const struct nv_window_area area)
{
    if (!root) {
        return NV_ERR_NOT_INIT;
    }

    if (root->kind == NV_WM_SPLIT) {
        struct nv_window_area child_area = { 0 };

        if (root->split.kind == NV_SPLIT_HORIZONTAL) {
            // top split
            child_area.h = root->split.ratio * area.h;
            child_area.w = area.w;
            child_area.x = area.x;
            child_area.y = area.y;
            (void)nv_draw_windows(root->split.left, child_area);
            
            // bottom split
            child_area.y += child_area.h;
            child_area.h = area.h - child_area.h;
            (void)nv_draw_windows(root->split.right, child_area);
        }
        else {
            // left split
            child_area.h = area.h;
            child_area.w = root->split.ratio * area.w;
            child_area.x = area.x;
            child_area.y = area.y;
            (void)nv_draw_windows(root->split.left, child_area);

            // right split
            child_area.x += child_area.w;
            child_area.w = area.w - child_area.w;
            (void)nv_draw_windows(root->split.right, child_area);
        }

        return NV_OK;
    }

    if (root->leaf.view) {          // TODO: this shouldn't be necessary here, all view drawing logic (null or not)
                                    // needa be moved into draw_view
        // FIXME: doing this only for ez compile, there's no reason 2 pass area in nv_draw_windows
        nv_draw_background_rect(area.x, area.y, area.x + area.w, area.y + area.h);
        nv_draw_view(root->leaf.view, &area);
    }

    return NV_OK;
}

// FIXME: really shitty code, difficult to understand, sometimes inefficient, potentially unsafe
static void nv_buffer_print_tree(nv_pool_index tree, struct nv_view* view, const struct nv_window_area* area)
{
    if (!view || !view->buffer) {
        return;
    }

    size_t line = view->top_line_index - 1;
    size_t lines_remaining = area->h; // how many lines after 'line' to flatten

    struct nv_tree_node* current = NODE_FROM_POOL(tree);

    while (current && lines_remaining > 0) {
        struct nv_tree_node* left = current->left ? NODE_FROM_POOL(current->left) : NULL;
        struct nv_tree_node* right = current->right ? NODE_FROM_POOL(current->right) : NULL;

        size_t left_lf = left ? left->data.lfcount : 0;
        size_t right_lf = right ? right->data.lfcount : 0;
        size_t local_lf = current->data.lfcount - left_lf - right_lf;

        if (line < left_lf) {
            // line in left tree
            current = left;
        }
        else if (line < left_lf + local_lf) {
            // line is within this node

            char* buf = nv_buffers[current->data.buff_id];
            size_t bufsiz = cvector_size(buf);
            // how many lines to skip to get the target line?
            size_t lines_to_skip = line - left_lf;
            
            struct nv_node line_node = {
                .buff_id = current->data.buff_id,
                .buff_index = current->data.buff_index,
                .length = 0,
                .lfcount = 1,
            };

            // skip necessary amt of lines
            while (line_node.buff_index < bufsiz && lines_to_skip > 0) {
                if (buf[line_node.buff_index] == '\n') {
                    lines_to_skip--;
                }
                line_node.buff_index++;
            }

            line_node.length = 0;
            size_t lines_collected = 0;

            // collect the rest of the lines in this node
            while (line_node.buff_index + line_node.length < bufsiz && lines_collected < lines_remaining) {
                if (buf[line_node.buff_index + line_node.length] == '\n') {
                    line_node.length++;
                    cvector_push_back(view->buffer->lines, line_node);
                    line_node.buff_index += line_node.length;
                    line_node.length = 0;
                    lines_collected++;
                } else {
                    line_node.length++;
                }
            }

            lines_remaining -= lines_collected;
            line = 0;
            current = right;
        }
        else {
            // line in right tree
            line -= left_lf + local_lf;
            current = right;
        }
    }
}

static int nv_draw_text_buffer(struct nv_view* view, const struct nv_window_area* area)
{
    if (!view || !view->buffer || !area) {
        return NV_ERR_NOT_INIT;
    }

    cvector_clear(view->buffer->lines); // FIXME, can reuse some lines depending on scroll shift
    nv_buffer_print_tree(view->buffer->tree, view, area);
    size_t computed_lines = cvector_size(view->buffer->lines);

    for (size_t line_no = view->top_line_index; line_no < view->top_line_index + area->h; line_no++) {
        if (line_no - view->top_line_index >= computed_lines) {
            break;
        }

        struct nv_node node = view->buffer->lines[line_no - view->top_line_index];
        nv_buffer_printf(view, area, line_no - view->top_line_index, line_no, &nv_buffers[node.buff_id][node.buff_index], node.length);
    }

    return NV_OK;
}

int netrw_filename_sort(const void* a, const void* b)
{
    return strcmp(*(const char**)a, *(const char**)b);
}

static int nv_draw_view(struct nv_view* view, const struct nv_window_area* area)
{
    if (!view || !view->buffer || !area) {
        return NV_ERR_NOT_INIT;
    }

    switch (view->buffer->type) {
    case NV_BUFF_TYPE_PLAINTEXT:
        nv_buffer_printf(view, area, 0, 0, view->buffer->buffer, area->w);

        break;

    case NV_BUFF_TYPE_SOURCE:
        if (!view->buffer->loaded) {
            view->gutter_width_cols = count_no_digits(view->buffer->line_count);
            view->buffer->loaded = true;
        }

        if (nv_draw_text_buffer(view, area) != NV_OK) {
            nv_fatal("failed to draw buffer");
        }

        break;

    case NV_BUFF_TYPE_BROWSER:
        // TODO: implement this as a plugin
        break;

    default:
        // TODO: log to log buffer
        // fprintf(stderr, "unsupported bufftype %d %f %f %f %f\n", ctx.buffer->type, window->w, window->h, window->x, window->y);
        break;
    }

    return NV_OK;
}

static int nv_calculate_statline()
{
    struct nv_context statline = nv_get_context(nv_editor->statline);
    struct nv_context focus = nv_get_context(nv_get_active_window());

    if (!statline.buffer || !focus.buffer) {
        return NV_ERR_NOT_INIT;
    }

    if (snprintf(statline.buffer->buffer, NV_BUFF_CHUNK_SIZE, "%s (%s, %s) --%s--"/* "%d %d,%d/%ld" */, focus.buffer->path,
                nv_str_buff_type[focus.buffer->type], nv_str_buff_fmt[focus.buffer->format],
                nv_mode_str[nv_editor->mode] /*, c.y, c.line, c.x, l ? l->data.length : 0 */) == -1) {
        return NV_ERR_MEM;
    }

    return NV_OK;
}
