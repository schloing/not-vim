#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "buffer.h"
#include "cvector.h"
#include "editor.h"
#include "error.h"
#include "nvtree/nvtree.h"
#include "view.h"

static size_t nv_calculate_tree_node_granularity(struct nv_buff* buff);
static void nv_render_lines_clear(struct nv_buff* buff);
static int nv_render_line_push(struct nv_buff* buff, const char* buf, size_t len);
static int nv_flatten_node_text(struct nv_buff* buff, const struct nv_node* data, size_t* lines_remaining, size_t* lines_to_skip, size_t* line_len);

bool is_elf(const char* buffer)
{
    const char e_ident[] = { 0x7f, 45, 0x4c, 46 };
    for (int i = 0; i < 4; i++) {
        if (e_ident[i] != buffer[i]) {
            return false;
        }
    }
    return true;
}

int nv_buffer_open_file(struct nv_buff* buff, const char* path)
{
    if (!buff || !path || !buff->buffer) {
        return NV_ERR_NOT_INIT;
    }

    struct stat sb;
    if (stat(buff->path, &sb) == -1) {
        return NV_ERR;
    }

    switch (sb.st_mode & S_IFMT) {
    case S_IFLNK: // symlink
    case S_IFDIR:
        buff->type = NV_BUFF_TYPE_BROWSER;
        break;

    case S_IFREG:
        buff->type = NV_BUFF_TYPE_SOURCE;

        if (access(buff->path, W_OK) == 0) {
            buff->file = fopen(buff->path, "rb+");
        }
        else {
            // TODO: set readonly indicator
            buff->file = fopen(buff->path, "rb");
        }

        if (buff->file == NULL) {
            return NV_ERR;
        }

        buff->bytes_loaded = fread(buff->buffer, sizeof(char), buff->chunk_size, buff->file);
        cvector_set_size(buff->buffer, buff->chunk_size);

        break;

    case S_IFSOCK:
        buff->type = NV_BUFF_TYPE_NETWORK;
        break;

    default:
        return NV_ERR;
    }

    return NV_OK;
}

// FIXME: receive BUFFTYPE instead of filepath, more flexible
struct nv_view* nv_view_init(const char* buffer_file_path)
{
    struct nv_view* view = (struct nv_view*)calloc(1, sizeof(struct nv_view));

    if (!view) {
        nv_editor->status = NV_ERR_MEM;
        return NULL;
    }

    view->top_line_index = 1;
    view->buffer = nv_buffer_init(buffer_file_path);
    view->gutter_gap = 1;

    static_assert(NV_CURSOR_CAP > NV_PRIMARY_CURSOR, "");
    cvector_reserve(view->cursors, NV_CURSOR_CAP);
    view->cursors[NV_PRIMARY_CURSOR] = (struct cursor) { .line = 1 };
    cvector_set_size(view->cursors, 1);

    if (nv_editor->status != NV_OK) {
        return NULL;
    }

    cvector_push_back(nv_editor->views, view);
    nv_editor->status = NV_OK;
    return view;
}

#define NV_TREE_MIN_CHARS_FOR_NODE_INIT 8
#define NV_LINES_PER_NODE               8

#ifdef NV_DEBUG_TREE_PRINT
static void print(nv_pool_index tree)
{
    struct nv_tree_node* node = NODE_FROM_POOL(tree);

    if (!node) {
        return;
    }

    print(node->left);

    size_t bufsize = cvector_size(nv_buffers[node->data.buff_id]);

    if (node->data.buff_index < bufsize &&
        node->data.buff_index + node->data.length <= bufsize) {
        printf("%.*s\n", (int)node->data.length, &nv_buffers[node->data.buff_id][node->data.buff_index]);
    }

    print(node->right);
}

static void print_current_only(nv_pool_index tree)
{
    struct nv_tree_node* node = NODE_FROM_POOL(tree);

    if (!node) {
        return;
    }

    size_t bufsize = cvector_size(nv_buffers[node->data.buff_id]);

    if (node->data.buff_index < bufsize &&
        node->data.buff_index + node->data.length <= bufsize) {
        printf("%.*s\n", (int)node->data.length, &nv_buffers[node->data.buff_id][node->data.buff_index]);
    }
}
#endif

#define NV_MIN_GRANULARITY 64
#define NV_MAX_GRANULARITY (64 * 1024)

static size_t nv_calculate_tree_node_granularity(struct nv_buff* buff)
{
    if (!buff) {
        nv_editor->status = NV_ERR_NOT_INIT;
        return 0;
    }

    if (buff->bytes_loaded == 0) {
        return NV_MIN_GRANULARITY;
    }

    size_t granularity = (buff->chunk_size * 1024) / buff->bytes_loaded;
    granularity *= 1024;

    if (granularity < NV_MIN_GRANULARITY) {
        granularity = NV_MIN_GRANULARITY;
    }
    else if (granularity > NV_MAX_GRANULARITY) {
        granularity = NV_MAX_GRANULARITY;
    }

    if (granularity > buff->chunk_size) {
        granularity = buff->chunk_size;
    }

    return granularity;
}

// FIXME: name does not indicate that it performs important set up for nvtree to work
int nv_buffer_build_tree(struct nv_buff* buff)
{
    if (!buff || !buff->buffer) {
        return NV_ERR_NOT_INIT;
    }

    char* b = buff->buffer;
    nv_buffers[buff->buff_id] = b;
    buff->tree = nv_tree_init();

    size_t line_count = 0;
    size_t abs_pos = 0;
    size_t tree_pos = 0;
    size_t granularity = nv_calculate_tree_node_granularity(buff);

    struct nv_node node = {
        // .buff_purpose = NV_BUFF_ID_ORIGINAL,
        .buff_id = buff->buff_id,
        .buff_index = 0,
        .length = 0,
        .length_left = 0,
        .lfcount = 0
    };

    while (abs_pos < buff->chunk_size) {
        if (!b[abs_pos]) {
            break;
        }

        node.length++;
        if (b[abs_pos] == '\n') {
            node.lfcount++;
            line_count++;
        }

        if (node.length >= granularity) {
            buff->tree = nv_tree_insert(buff->tree, tree_pos, node);
            buff->tree = nv_tree_paint(buff->tree, B);
            tree_pos += node.length;
            node.buff_index += node.length;
            node.length = 0;
            node.lfcount = 0;
        }

        abs_pos++;
    }

    if (node.length > 0) {
        buff->tree = nv_tree_insert(buff->tree, tree_pos, node);
        buff->tree = nv_tree_paint(buff->tree, B);
    }

    if (line_count == 0) {
        line_count = 1;
    }

    buff->line_count = line_count;

    return NV_OK;
}

struct nv_buff* nv_buffer_init(const char* path)
{
    struct nv_buff* buffer = (struct nv_buff*)calloc(1, sizeof(struct nv_buff));

    if (!buffer) {
        nv_editor->status = NV_ERR_NOT_INIT;
        return NULL;
    }

#define NV_BUFF_RENDER_BUFF_SIZE      4096
#define NV_BUFF_INT_SCRATCH_BUFF_SIZE 256

    static size_t buff_id;
    buffer->type = NV_BUFF_TYPE_PLAINTEXT;
    buffer->chunk_size = NV_BUFF_CHUNK_SIZE;
    buffer->buff_id = buff_id++;
    cvector_reserve(buffer->renders, NV_BUFF_RENDER_BUFF_SIZE);
    cvector_reserve(buffer->scratch, NV_BUFF_INT_SCRATCH_BUFF_SIZE);
    cvector_reserve(buffer->buffer, (size_t)NV_BUFF_CHUNK_SIZE);
    cvector_reserve(buffer->add_buffer, (size_t)NV_BUFF_CHUNK_SIZE);
    cvector_reserve(buffer->lines, (size_t)NV_LINE_CAP);

    if (path) {
        buffer->path = (char*)path;
        nv_editor->status = nv_buffer_open_file(buffer, path);
        (void)nv_buffer_build_tree(buffer);
    }

    return nv_editor->status == NV_OK ? buffer : NULL;
}

nv_pool_index line(struct nv_context* ctx, int lineno)
{
    if (!ctx || !ctx->buffer) {
        return NV_NULL_INDEX;
    }

    return nv_find_by_line(ctx->buffer->tree, lineno, NULL);
}

struct nv_render_line* nv_get_computed_line(struct nv_context* ctx, int lineno)
{
    if (!ctx || !ctx->buffer || !ctx->view) {
        return NULL;
    }

    if (lineno - ctx->view->top_line_index < 0) {
        return NULL;
    }

    if (cvector_size(ctx->buffer->lines) <= lineno - ctx->view->top_line_index) {
        return NULL;
    }

    return &ctx->buffer->lines[lineno - ctx->view->top_line_index];
}

int nv_free_view(struct nv_view* view)
{
    if (!view) {
        return NV_ERR_NOT_INIT;
    }

    cvector_free(view->cursors);
    nv_free_buffer(view->buffer);

    free(view);
    return NV_OK;
}

int nv_free_buffer(struct nv_buff* buff)
{
    if (!buff) {
        return NV_ERR_NOT_INIT;
    }

    if (buff->file) {
        (void)fclose(buff->file);
        buff->file = NULL;
    }

    if (buff->tree != NV_NULL_INDEX) {
        nv_tree_free_all(buff->tree);
    }

    cvector_free(buff->lines);
    cvector_free(buff->renders);
    cvector_free(buff->scratch);
    cvector_free(buff->buffer);
    cvector_free(buff->add_buffer);

    free(buff);
    return NV_OK;
}

static void nv_render_lines_clear(struct nv_buff* buff)
{
    if (!buff) {
        return;
    }

    cvector_clear(buff->lines);
    cvector_clear(buff->renders);
    cvector_clear(buff->scratch);
}

static int nv_render_line_push(struct nv_buff* buff, const char* buf, size_t len)
{
    if (!buff) {
        return NV_ERR_NOT_INIT;
    }

    struct nv_render_line line = { 0 };

    size_t start = cvector_size(buff->renders);
    cvector_reserve(buff->renders, start + len + 1);
    cvector_set_size(buff->renders, start + len + 1);

    if (len > 0) {
        memcpy(&buff->renders[start], buf, len);
    }
    buff->renders[start + len] = '\0';

    line.text = &buff->renders[start];
    line.length = len;
    cvector_push_back(buff->lines, line);

    return NV_OK;
}

static int nv_flatten_node_text(struct nv_buff* buff, const struct nv_node* data, size_t* lines_remaining, size_t* lines_to_skip, size_t* line_len)
{
    if (!buff || !data || data->length == 0 || !lines_remaining || !lines_to_skip || !line_len) {
        return NV_ERR_NOT_INIT;
    }

    char* buf = nv_buffers[data->buff_id];
    if (!buf) {
        return NV_ERR_NOT_INIT;
    }

    size_t start = data->buff_index;
    size_t end = start + data->length;

    for (size_t i = start; i < end && *lines_remaining > 0; i++) {
        char c = buf[i];

        if (c == '\0') {
            break;
        }

        if (c == '\r') {
            if (i + 1 < end && buf[i + 1] == '\n') {
                i++;
            }
            c = '\n';
        }

        if (c != '\n') {
            if (*lines_to_skip > 0) {
                continue;
            }

            cvector_push_back(buff->scratch, c);
            (*line_len)++;
            continue;
        }

        if (*lines_to_skip > 0) {
            (*lines_to_skip)--;
        } else {
            if (nv_render_line_push(buff, buff->scratch, *line_len) != NV_OK) {
                return NV_ERR;
            }
            (*lines_remaining)--;
        }

        *line_len = 0;
        cvector_clear(buff->scratch);
    }

    return NV_OK;
}

void nv_buffer_flatten_tree(nv_pool_index tree, struct nv_view* view, const struct nv_window_area* area)
{
    if (!view || !view->buffer || tree == NV_NULL_INDEX || !area || area->h == 0) {
        return;
    }

    struct nv_buff* buff = view->buffer;
    nv_render_lines_clear(buff);

    nv_pool_index stack[NVTREE_MAX_STACK_DEPTH];
    int top = 0;
    nv_pool_index current = tree;

    size_t lines_remaining = area->h;
    size_t lines_to_skip = view->top_line_index > 0 ? view->top_line_index - 1 : 0;
    size_t line_len = 0;

    while ((current != NV_NULL_INDEX || top > 0) && lines_remaining > 0) {
        while (current != NV_NULL_INDEX) {
            if (top >= NVTREE_MAX_STACK_DEPTH) {
                return;
            }

            stack[top++] = current;

            struct nv_tree_node* n = NODE_FROM_POOL(current);
            if (!n) {
                current = NV_NULL_INDEX;
                break;
            }

            current = n->left;
        }

        if (top == 0) {
            break;
        }

        current = stack[--top];
        struct nv_tree_node* node = NODE_FROM_POOL(current);
        if (!node) {
            current = NV_NULL_INDEX;
            continue;
        }

        if (nv_flatten_node_text(buff, &node->data, &lines_remaining, &lines_to_skip, &line_len) != NV_OK) {
            return;
        }

        current = node->right;
    }

    if (lines_remaining > 0 && lines_to_skip == 0) {
        if (line_len > 0 || cvector_size(buff->lines) == 0) {
            (void)nv_render_line_push(buff, buff->scratch, line_len);
        }
    }
}