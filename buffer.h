#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cursor.h"
#include "cvector.h"

struct nv_conf {
    int  tab_width;
    bool expand_tab;
    bool auto_indent;
    bool line_numbers;
    bool show_relative;
    int  command_delay;
    int  status_height;
    bool show_status;
    bool show_buffer;
    bool show_headless;
};

enum nv_bufftype {
    NV_BUFFTYPE_STDIN     = 1,
    NV_BUFFTYPE_STDOUT    = 2,
    NV_BUFFTYPE_BROWSER   = 4,
    NV_BUFFTYPE_NETWORK   = 8,
    NV_BUFFTYPE_SOURCE    = 16,
    NV_BUFFTYPE_PLAINTEXT = 32,
};

enum nv_bufffmt {
    NV_FILE_FORMAT_BINARY,
    NV_FILE_FORMAT_SOURCE,  // lsp + treesitter impl
    NV_FILE_FORMAT_PLAINTEXT,
};

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
    enum nv_bufffmt  format;
    FILE*  file;
    char*  path;
    size_t chunk;
    bool   loaded;
    struct {
        size_t top_line;
        size_t linecol_size;
        size_t line_count;
    };
    cvector(char)  buffer;
    cvector(struct nv_buff_line) lines;
    cvector(struct cursor) cursors;
};

struct nv_buff_line* currline(struct nv_buff* buff);
struct nv_buff_line* prevline(struct nv_buff* buff);
struct nv_buff_line* nextline(struct nv_buff* buff);
struct nv_buff_line* line(struct nv_buff* buff, size_t lineno);
size_t nv_get_line_length(struct nv_buff_line line);
void nv_buffer_init(struct nv_buff* buff, char* path);
void nv_load_file_buffer(struct nv_buff* buffer, size_t* out_line_count);

#endif
