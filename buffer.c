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

void nv_buffer_init(struct nv_buff* buff, char* path) {
    NV_ASSERT(path);
    NV_ASSERT(buff);

#define NV_BUFFID_UNSET 0
    buff->id     = NV_BUFFID_UNSET;
#define NV_BUFF_CAP     1024 * 2
    buff->buffer = malloc(NV_BUFF_CAP);
    buff->chunk  = vector_capacity(buff->buffer); // should be NV_BUFF_CAP
    buff->lines  = vector_create();
    
    buff->path   = path;
    buff->file   = fopen(buff->path, "r+");
    // TODO: create file at path (because it does not exist?)
    if (buff->file == NULL) return;
   
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
        return;
    }
}

void _nv_load_file_buffer(struct nv_buff* buffer) {
    char* b = buffer->buffer;
    struct nv_buff_line line = { 0 };
    struct nv_buff_line* l;

    int line_count = 0;
    int i = 0;
    while (b[i++] != '\0') {
        if (b[i] == '\n') {
            line.end = i;
            l = vector_add_dst(&buffer->lines); // add line into buffer->lines using temporary pointer
            l->end   = line.end;
            l->begin = line.begin;

            size_t size = line.end - line.begin;
            char* line_string = malloc(size + 1);
            memcpy(line_string, b + line.begin, size);
            line_string[size] = '\0';
            tb_print(0, line_count, TB_WHITE, TB_BLACK, line_string);
            free(line_string);

            line.end = -1;
            line.begin = i + 1;
            line_count++;
        }
    }

    l = NULL;
}

void nv_free_buffers(struct nv_editor* editor) {
    NV_ASSERT(editor->buffers);
    for (size_t i = 0; i < vector_size(editor->buffers); i++) {
        if (editor->buffers[i].file != NULL)
            fclose(editor->buffers[i].file);
        vector_free(editor->buffers[i].lines);
        free(editor->buffers[i].buffer);
        editor->buffers[i].buffer = NULL;
    }
    vector_free(editor->buffers);
    editor->buffers = NULL;
}
