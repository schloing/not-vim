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
#include "termbox2.h"
#include "view.h"

static size_t nv_calculate_tree_node_granularity(struct nv_buff* buff);

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

    size_t granularity = (buff->chunk_size * 1024) / buff->bytes_loaded;

    if (granularity < 0) {
        granularity = 1;
    }

    granularity = granularity * 1024;

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

    static size_t buff_id;
    buffer->type = NV_BUFF_TYPE_PLAINTEXT;
    buffer->chunk_size = NV_BUFF_CHUNK_SIZE;
    buffer->buff_id = buff_id++;
    cvector_reserve(buffer->buffer, (size_t)NV_BUFF_CHUNK_SIZE);
    cvector_reserve(buffer->add_buffer, (size_t)NV_BUFF_CHUNK_SIZE); // TODO: determine size to allocate here
    cvector_reserve(buffer->lines, (size_t)NV_LINE_CAP);

    if (path) {
        buffer->path = (char*)path;
        nv_editor->status = nv_buffer_open_file(buffer, path);
        nv_buffer_build_tree(buffer); // only fails when nv_buff or nv_buff->buffer is NULL
    }

    return nv_editor->status == NV_OK ? buffer : NULL;
}

nv_pool_index line(struct nv_context* ctx, int lineno)
{
    if (!ctx || !ctx->buffer) {
        return NV_NULL_INDEX;
    }

    return nv_find_by_line(ctx->buffer->tree, lineno);
}

struct nv_node* nv_get_computed_line(struct nv_context* ctx, int lineno)
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
    cvector_free(buff->buffer);
    cvector_free(buff->add_buffer);

    free(buff);
    return NV_OK;
}

// FIXME: really shitty code, difficult to understand, sometimes inefficient, potentially unsafe
void nv_buffer_flatten_tree(nv_pool_index tree, struct nv_view* view, const struct nv_window_area* area)
{
    if (!view || !view->buffer) {
        return;
    }

#ifdef NV_DEBUG_INEFFICIENT_TREE_FLATTEN
    for (int line = view->top_line_index - 1; line < line + area->h; line++) {
        nv_tree_pool_index l = nv_find_by_line(view->buffer->tree, line);
        if (l == NV_NULL_INDEX) {
            break;
        }
        cvector_push_back(view->buffer->lines, NODE_FROM_POOL(l)->data);
    }

    return;
#endif

    size_t line = view->top_line_index - 1;
    size_t lines_remaining = area->h; // how many lines after 'line' to flatten
    bool searching_for_top = true;
    struct nv_tree_node* current = NODE_FROM_POOL(tree);

    while (current && lines_remaining > 0) {
        struct nv_tree_node* left = current->left ? NODE_FROM_POOL(current->left) : NULL;
        struct nv_tree_node* right = current->right ? NODE_FROM_POOL(current->right) : NULL;

        size_t left_lf = left ? left->data.lfcount : 0;
        size_t right_lf = right ? right->data.lfcount : 0;
        size_t local_lf = current->data.lfcount - left_lf - right_lf;

        if (left_lf == 0 && right_lf == 0 && local_lf == 0 && line == 0) {
            // edge case - there is only 1 line
            cvector_push_back(view->buffer->lines, current->data);
            break;
        }

        if (line < left_lf) {
            // line in left tree
            current = left;
        }
        else if (searching_for_top ? (line < left_lf + local_lf) : false) {
            searching_for_top = false; // this node has top_line_index

            // line is within this node
            char* buf = nv_buffers[current->data.buff_id];
            if (!buf) {
                return;
            }

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

            size_t lines_collected = 0;
            // collect the rest of the lines in this node
            while (line_node.buff_index < bufsiz && lines_collected < lines_remaining) {
                if (line_node.buff_index + line_node.length >= bufsiz) {
                    return;
                }

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