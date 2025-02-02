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

enum nv_buff_t {
    NV_BUFFTYPE_STDIN     = 1,
    NV_BUFFTYPE_STDOUT    = 2,
    NV_BUFFTYPE_BROWSER   = 4,
    NV_BUFFTYPE_NETWORK   = 8,
    NV_BUFFTYPE_SOURCE    = 16,
    NV_BUFFTYPE_PLAINTEXT = 32,
};

enum nv_buff_f {
    NV_FILE_FORMAT_BINARY,
    NV_FILE_FORMAT_SOURCE,  // lsp + treesitter impl
    NV_FILE_FORMAT_PLAINTEXT,
};

struct nv_buff_line {
    size_t begin;
    size_t end;
    size_t length;
};

struct nv_window {
    int x;
    int y;
    int w;
    int h;
    size_t buff_id;
};

#define NV_BUFFID_UNSET 0
#define NV_BUFF_CAP     1024 * 16
#define NV_LINE_CAP     32

struct nv_buff {
    size_t          id;
    enum nv_buff_t  type;
    enum nv_buff_f  format;
    FILE*           file;
    char*           path;
    size_t          chunk;
    bool            loaded;
    int             _begin_line;
    int             _lines_col_size;
    int             _line_count;
    cvector(char) buffer;
    cvector(struct nv_buff_line) lines;
    cvector(struct cursor) cursors;
};

struct nv_buff_line* currline(struct nv_buff* buff);
struct nv_buff_line* prevline(struct nv_buff* buff);
struct nv_buff_line* nextline(struct nv_buff* buff);
struct nv_buff_line* line(struct nv_buff* buff, int lineno);
size_t nv_get_line_length(struct nv_buff_line line);
void nv_buffer_init(struct nv_buff* buff, char* path);
void nv_load_file_buffer(struct nv_buff* buffer, int* out_line_count);

#endif
