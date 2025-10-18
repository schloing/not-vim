#ifndef NV_BUFFER_H
#define NV_BUFFER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cursor.h"
#include "cvector.h"
#include "window.h"

#define NV_BUFFID_UNSET 0
#define NV_BUFF_CAP     1024 * 16
#define NV_LINE_CAP     32

enum nv_bufftype {
    NV_BUFFTYPE_STDIN        = 0,
    NV_BUFFTYPE_STDOUT       = 1,
    NV_BUFFTYPE_BROWSER      = 2,
    NV_BUFFTYPE_NETWORK      = 3,
    NV_BUFFTYPE_SOURCE       = 4,
    NV_BUFFTYPE_PLAINTEXT    = 5,
    NV_BUFFTYPE_LOG          = 6,
};

enum nv_bufffmt {
    NV_FILE_FORMAT_BINARY    = 0,
    NV_FILE_FORMAT_SOURCE    = 1, // lsp + treesitter impl
    NV_FILE_FORMAT_PLAINTEXT = 2,
};

extern char* nv_bufftype_str[NV_BUFFTYPE_LOG + 1];
extern char* nv_bufffmt_str[NV_BUFFTYPE_PLAINTEXT + 1];

struct nv_buff_line {
    size_t begin;
    size_t end;
    size_t length;
};

struct nv_visual_row {
    size_t line_index;
    size_t wrap_index;
    size_t offset;
};

#define NV_MAP_CAP 128

struct nv_view {
    size_t top_line_index;
    int gutter_digit_width;
    int line_count;
    struct nv_buff* buffer;
    cvector(struct nv_visual_row) visual_rows;
    cvector(size_t) map; // enough space to map ~100 lines to visual rows
    cvector(struct cursor) cursors;
};

struct nv_buff {
    FILE* file;
    char* path;
    size_t chunk;
    bool loaded;
    enum nv_bufftype type;
    enum nv_bufffmt format;
    cvector(struct nv_buff_line) lines;
    cvector(char) buffer;
};

struct nv_view* nv_view_init(const char* buffer_file_path);
struct nv_buff* nv_buffer_init(const char* path);
int nv_buffer_open_file(struct nv_buff* buff, const char* path);
int nv_rebuild_lines(struct nv_buff* buff, int* out_line_count);
int nv_free_view(struct nv_view* view);
int nv_free_buffer(struct nv_buff* buff);
struct nv_buff_line* line(struct nv_context* ctx, size_t lineno);

#endif
