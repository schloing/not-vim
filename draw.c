#include "buffer.h"
#include "cursor.h"
#include "cvector.h"
#include "draw.h"
#include "editor.h"
#include "events.h"
#include "error.h"
#include "view.h"
#include "window.h"
#include <tui.h>

static int count_no_digits(int n);

void nv_draw_background_rect(int x1, int y1, int x2, int y2)
{
#ifdef NV_DEBUG_WINDOW_BACKGROUND_COLOURS
    int colour = rand() % 0x1000000;
#endif

    for (int i = x1; i < x2; i++) {
        for (int j = y1; j < y2; j++) {
#ifdef NV_DEBUG_WINDOW_BACKGROUND_COLOURS
            nv_tui_set_cell(i, j, ' ', nv_editor->config.fg_main, colour);
#else
            nv_tui_set_cell(i, j, ' ', NV_TUI_HL_BLACK_ON_WHITE);
#endif
        }
    }

    // caller needa call tb_present()
}

void nv_draw()
{
    nv_tui_present();
}

void nv_draw_background()
{
    nv_tui_clear();
}

void nv_draw_cursor()
{
    struct nv_context ctx = nv_get_context(nv_get_focused_window());
    struct cursor c;

    if (!ctx.view || !ctx.buffer) {
        // FIXME
        return;
    }

    int effective_row = 0;
    struct nv_render_line* l;

    for (int cindex = 0; cindex < cvector_size(ctx.view->cursors); cindex++) {
        c = ctx.view->cursors[cindex];
        if (c.line > ctx.buffer->line_count) {
            continue;
        }

        l = nv_get_computed_line(&ctx, c.line);

        if (!l) {
            continue;
        }

        effective_row =
            ctx.window->leaf.area.x +                                   // window position
            ctx.view->gutter_width_cols + ctx.view->gutter_gap +        // space taken by line numbers
            (c.x > l->length ? l->length : c.x);                        // cap the cursor to the end of the line

        nv_tui_invert_cell(effective_row, c.line - ctx.view->top_line_index);
    }
}

int nv_draw_windows(struct nv_window_node* root, const struct nv_window_area area)
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
        root->leaf.area = area;
        nv_draw_view(root->leaf.view, &area);
    }

    return NV_OK;
}

int nv_draw_text_buffer(struct nv_view* view, const struct nv_window_area* area)
{
    if (!view || !view->buffer || !area) {
        return NV_ERR_NOT_INIT;
    }

    cvector_clear(view->buffer->lines); // FIXME, can reuse some lines depending on scroll shift
    nv_buffer_flatten_tree(view->buffer->tree, view, area);
    size_t computed_lines = cvector_size(view->buffer->lines);
    size_t line_no = view->top_line_index;
#define VIEW_DRAWABLE_WIDTH (area->w - (view->gutter_gap + view->gutter_width_cols))
#define RELATIVE_LINE_INDEX (line_no - view->top_line_index)
    for (size_t row = 0; row < area->h;) {
        if (RELATIVE_LINE_INDEX > computed_lines) {
            break;
        }

        struct nv_render_line line = view->buffer->lines[RELATIVE_LINE_INDEX];

        if (line.length > 0) {
            for (int i = 0; i < line.length; i += VIEW_DRAWABLE_WIDTH) {
                if (view->top_line_index + row > view->buffer->line_count) {
                    break;
                }
                nv_buffer_printf(view, area, row, line_no, line.text + i, VIEW_DRAWABLE_WIDTH);
                row++;
            }
        }
        else {
            nv_buffer_printf(view, area, row, line_no, line.text, 0);
            row++;
        }

        line_no++;
    }

    return NV_OK;
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

int nv_draw_view(struct nv_view* view, const struct nv_window_area* area)
{
    if (!view || !view->buffer || !area) {
        return NV_ERR_NOT_INIT;
    }

    // TODO: indicate window = NULL?
    struct nv_context ctx = {
        .window = NULL,
        .view = view,
        .buffer = view->buffer,
    };

    nv_event_emit(NV_EVENT_BUFFDRAW_START, &ctx);

    switch (view->buffer->type) {
    case NV_BUFF_TYPE_LOG:
    case NV_BUFF_TYPE_PLAINTEXT: {
        char *p = view->buffer->buffer, *nl = NULL;
        size_t lines_read = 0;

        while ((nl = strchr(p, '\n')) != NULL) {
            if (lines_read <= view->buffer->line_count && lines_read < area->h) {
                nv_buffer_printf(view, area, lines_read, lines_read, p, nl - p);
            }
            p = nl + 1;
            lines_read++;
        }

        if (*p) {
            nv_buffer_printf(view, area, lines_read, 0, p, area->w);
        }

        break;
    }
    case NV_BUFF_TYPE_SOURCE:
        view->gutter_width_cols = count_no_digits(view->buffer->line_count);

        if (nv_draw_text_buffer(view, area) != NV_OK) {
            nv_fatal("failed to draw buffer");
        }

        break;

    case NV_BUFF_TYPE_BROWSER:
        // TODO: implement this as a plugin

    default:
        nv_log("unsupported bufftype %d\n", view->buffer->type);
        nv_event_emit(NV_EVENT_BUFFDRAW_UNSUPPORTED, &ctx);
        break;
    }

    nv_event_emit(NV_EVENT_BUFFDRAW_END, &ctx);
    return NV_OK;
}

#define NV_PRINTF(x, y, fg, fmt, ...) \
    nv_tui_printf(x, y, fg, fmt, ##__VA_ARGS__)

void nv_buffer_printf(struct nv_view* view, const struct nv_window_area* area, int row, int line_no, char* lbuf, size_t length)
{
    if (!lbuf || !view || length > area->w) {
        return;
    }

    size_t gutter_offset = 0;
    char* string = (char*)malloc(length + 1);
    (void)stpncpy(string, lbuf, length);
    string[length] = '\0';

    if (view->gutter_width_cols > 0) {
        gutter_offset = view->gutter_width_cols + view->gutter_gap;
        NV_PRINTF(area->x, area->y + row, NV_TUI_HL_WHITE_ON_BLACK, "%*d", view->gutter_width_cols, line_no);
    }

    NV_PRINTF(area->x + gutter_offset, area->y + row, NV_TUI_HL_WHITE_ON_BLACK, "%s", string);

    free(string);
}

int nv_calculate_statline()
{
    struct nv_context statline = nv_get_context(nv_editor->statline);
    struct nv_context focus = nv_get_context(nv_get_focused_window());

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
