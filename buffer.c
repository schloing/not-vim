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
        buff->file = fopen(buff->path, "rb+");

        if (buff->file == NULL) {
            return NV_ERR;
        }

        fread(buff->buffer, sizeof(char), buff->chunk, buff->file);
        cvector_set_size(buff->buffer, buff->chunk);

        break;

    case S_IFSOCK:
        buff->type = NV_BUFF_TYPE_NETWORK;
        break;

    default:
        return NV_ERR;
    }

    return NV_OK;
}

struct nv_view* nv_view_init(const char* buffer_file_path)
{
    struct nv_view* view = (struct nv_view*)calloc(1, sizeof(struct nv_view));

    if (!view) {
        nv_editor->status = NV_ERR_MEM;
        return NULL;
    }

    view->top_line_index = 0;
    view->buffer = nv_buffer_init(buffer_file_path);

    static_assert(NV_CURSOR_CAP > NV_PRIMARY_CURSOR, "");
    cvector_reserve(view->cursors, NV_CURSOR_CAP);
    view->cursors[NV_PRIMARY_CURSOR] = (struct cursor) { 0 };
    
    if (nv_editor->status != NV_OK) {
        return NULL;
    }

    cvector_reserve(view->map, (size_t)NV_MAP_CAP);

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

// FIXME: name does not indicate that it performs important set up for nvtree to work
int nv_buffer_build_tree(struct nv_buff* buff)
{
    if (!buff || !buff->buffer) {
        return NV_ERR_NOT_INIT;
    }

    char* b = buff->buffer;

    nv_buffers[NV_BUFF_ID_ORIGINAL] = b;

    buff->tree = nv_tree_init();

    struct nv_node node = {
        .buff_id = NV_BUFF_ID_ORIGINAL,
        .buff_index = 0,
        .length = buff->chunk,
        .length_left = 0,
        .lfcount = 0
    };

    buff->tree = nv_tree_insert(buff->tree, 0, node);
    buff->tree = nv_tree_paint(buff->tree, B);

    struct nv_tree_node* tree = NODE_FROM_POOL(buff->tree);

    buff->line_count = tree->data.lfcount;

    return NV_OK;
}

struct nv_buff* nv_buffer_init(const char* path)
{
    struct nv_buff* buffer = (struct nv_buff*)calloc(1, sizeof(struct nv_buff));

    if (!buffer) {
        nv_editor->status = NV_ERR_NOT_INIT;
        return NULL;
    }

    buffer->chunk = NV_BUFF_CHUNK_SIZE;
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

int nv_free_view(struct nv_view* view)
{
    if (!view) {
        return NV_ERR_NOT_INIT;
    }

    cvector_free(view->visual_rows);
    cvector_free(view->cursors);
    cvector_free(view->map);
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

    if (buff->tree) {
        nv_tree_free_all(buff->tree);
    }

    cvector_free(buff->lines);
    cvector_free(buff->buffer);
    cvector_free(buff->add_buffer);

    free(buff);
    return NV_OK;
}
