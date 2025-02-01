#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <stdbool.h>

#include "bufferhelp.h"
#include "cursor.h"
#include "cvector.h"

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
    struct nv_buff_internal internal;
    cvector_vector_type(struct cursor) cursors;
};

struct nv_buff_line* currline(struct nv_buff* buff);
struct nv_buff_line* nextline(struct nv_buff* buff);
struct nv_buff_line* line(struct nv_buff* buff, int lineno);
void nv_buffer_init(struct nv_buff* buff, char* path);
void _nv_load_file_buffer(struct nv_buff* buffer, int* out_line_count);

#endif
