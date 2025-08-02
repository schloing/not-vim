#ifndef NV_BUFFER_H
#define NV_BUFFER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "cursor.h"
#include "cvector.h"

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

#define NV_BUFFID_UNSET 0
#define NV_BUFF_CAP     1024 * 16
#define NV_LINE_CAP     32

struct nv_buff {
    enum nv_bufftype type;
    enum nv_bufffmt format;
    FILE* file;
    char* path;
    size_t chunk;
    bool loaded;
    struct {
        size_t top_line;
        size_t linecol_size;
        size_t line_count;
    };
    cvector(char) buffer;
    cvector(struct nv_buff_line) lines;
    cvector(struct cursor) cursors;
};

struct nv_buff* nv_buffer_init(const char* path);
int nv_buffer_open_file(struct nv_buff* buff, const char* path);
int nv_load_file_buffer(struct nv_buff* buffer, size_t* out_line_count);
int nv_free_buffer(struct nv_buff* buff);
struct nv_buff_line* line(struct nv_buff* buff, size_t lineno);

#endif
