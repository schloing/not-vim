#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "assert.h"
#include "buffer.h"
#include "editor.h"
#include "vec.h"
#include "termbox2.h"

struct nv_buff* nv_buffer_init(char* path) {
    NV_ASSERT(path);
    struct nv_buff* buff = (struct nv_buff*)malloc(sizeof(struct nv_buff));
    NV_ASSERT(buff);

#define NV_BUFFID_UNSET 0
    buff->id     = NV_BUFFID_UNSET;
#define NV_BUFF_CAP     1024 * 2
    buff->path   = path;
    buff->buffer = vector_create();
    vector_reserve(&buff->buffer, NV_BUFF_CAP);
    buff->chunk  = vector_capacity(buff->buffer); // should be NV_BUFF_CAP
    buff->file   = fopen(path, "r+");
    NV_ASSERT(buff->file);
   
    struct stat sb;
    fstat(fileno(buff->file), &sb);
    
    switch (sb.st_mode & S_IFMT) {
    case S_IFLNK: // symlink
    case S_IFDIR:
        buff->type = NV_BUFFTYPE_BROWSER;
        break;
    
    case S_IFREG:
        buff->type = NV_BUFFTYPE_SOURCE;
        fread(buff->buffer, sizeof(char), buff->chunk, buff->file);
        break;
    
    case S_IFSOCK:
        buff->type = NV_BUFFTYPE_NETWORK;
        break;
    
    default:
        free(buff);
        return NULL;
    }

    return buff;
}

void _nv_load_file_buffer(struct nv_buff* buffer) {
    int row = 0;
    buffer->line = 0;
#define LINE_BUFF_SIZE 256
    char linebuff[LINE_BUFF_SIZE];
    char* b = buffer->buffer;

//#define TB_DRAW_LINE(buffer, linebuff) tb_printf(0, buffer->line, TB_WHITE, TB_BLACK, "%-4d %s", buffer->line, linebuff)
#define TB_DRAW_LINE(buffer, linebuff) tb_printf(0, buffer->line, TB_WHITE, TB_BLACK, "%s", linebuff)

    while (*b != '\0') {
        linebuff[row++] = *b;

        if (*b == '\n') {
            linebuff[row - 1] = '\0';

            TB_DRAW_LINE(buffer, linebuff);

            row = 0;
            buffer->line++;
        }

        b++;
    }

    if (row > 0) {
        linebuff[row] = '\0';
        TB_DRAW_LINE(buffer, linebuff);
    }

    buffer->loaded = true;
}

void nv_free_buffers(struct nv_editor* editor) {
    NV_ASSERT(editor->buffers);
    vector_free(editor->buffers);
    editor->buffers = NULL;
}
