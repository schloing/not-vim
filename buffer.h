#ifndef NV_BUFFER_H
#define NV_BUFFER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cursor.h"
#include "cvector.h"
#include "window.h"
#include "nvtree/nvtree.h"

#define NV_BUFFID_UNSET        0
#define NV_BUFF_CHUNK_SIZE     1024 * 16
#define NV_LINE_CAP            32

typedef enum {
    NV_BUFF_TYPE_STDIN        = 0,
    NV_BUFF_TYPE_STDOUT       = 1,
    NV_BUFF_TYPE_BROWSER      = 2,
    NV_BUFF_TYPE_NETWORK      = 3,
    NV_BUFF_TYPE_SOURCE       = 4,
    NV_BUFF_TYPE_PLAINTEXT    = 5,
    NV_BUFF_TYPE_LOG          = 6,
    NV_BUFF_TYPE_END,
} nv_buff_type;

typedef enum {
    NV_FILE_FORMAT_BINARY    = 0,
    NV_FILE_FORMAT_SOURCE    = 1, // lsp + treesitter impl
    NV_FILE_FORMAT_PLAINTEXT = 2,
    NV_FILE_FORMAT_END,
} nv_buff_fmt;

extern char* nv_str_buff_type[NV_BUFF_TYPE_END];
extern char* nv_str_buff_fmt[NV_FILE_FORMAT_END];

struct nv_view {
    size_t top_line_index;
    size_t padding;
    size_t gutter_gap;
    size_t gutter_width_cols;
    bool visible;
    bool allow_split;
    struct nv_buff* buffer;
    cvector(struct cursor) cursors;
};

typedef nv_pool_index nv_tree_pool_index;

struct nv_buff {
    FILE* file;
    char* path;
    bool loaded;
    int line_count;
    size_t buff_id;
    size_t chunk;
    nv_buff_type type;
    nv_buff_fmt format;
    nv_tree_pool_index tree;
    cvector(struct nv_node) lines;
    cvector(char) buffer;
    cvector(char) add_buffer;
};

struct nv_view* nv_view_init(const char* buffer_file_path);
struct nv_buff* nv_buffer_init(const char* path);
int nv_buffer_build_tree(struct nv_buff* buff);
int nv_buffer_open_file(struct nv_buff* buff, const char* path);
int nv_rebuild_lines(struct nv_buff* buff, int* out_line_count);
int nv_free_view(struct nv_view* view);
int nv_free_buffer(struct nv_buff* buff);
nv_pool_index line(struct nv_context* ctx, int lineno);

#endif
