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

#define NV_BUFFID_UNSET 0
#define NV_BUFF_CAP     1024 * 2

bool is_elf(char* buffer) {
    const char e_ident[] = { 0x7f, 45, 0x4c, 46 };
    for (int i = 0; i < 4; i++)
        if (e_ident[i] != buffer[i]) return false;
    return true;
}

void nv_buffer_init(struct nv_buff* buff, char* path) {
    NV_ASSERT(buff);

    buff->buffer = vector_create();
    vector_reserve(&buff->buffer, NV_BUFF_CAP);
    buff->chunk  = vector_capacity(buff->buffer); // should be NV_BUFF_CAP
    buff->lines  = vector_create();
    if (path == NULL) return;
    buff->path   = path;

    struct stat sb;
    if (stat(buff->path, &sb) == -1) return;

    switch (sb.st_mode & S_IFMT) {
    case S_IFLNK: // symlink
    case S_IFDIR:
        buff->type = NV_BUFFTYPE_BROWSER;
        break;
    
    case S_IFREG:
        buff->type = NV_BUFFTYPE_SOURCE;
        buff->file = fopen(buff->path, "rb+");
        if (buff->file == NULL) return;

        fread(buff->buffer, sizeof(char), buff->chunk, buff->file);

        if (is_elf(buff->buffer)) {
            // elf check happens before extension
            buff->file_format = NV_FILE_FORMAT_BINARY;
        }
        else {
            // TODO
            break;

            int i = 0;
            int dot = 0;
  
            while (buff->path[i] != '\0') {
                if (buff->path[i] == '.') dot = i;
                i++;
            }
  
            // buff->path + dot is extension
            char* extension = buff->path + dot;
        }

        break;
    
    case S_IFSOCK:
        buff->type = NV_BUFFTYPE_NETWORK;
        break;
    
    default:
        return;
    }
}

void _nv_load_file_buffer(struct nv_buff* buffer, int* out_line_count) {
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
            line.end = -1;                      // not necessary
            line.begin = i + 1;
            line_count++;
        }
    }

    l = NULL;
    *out_line_count = line_count;
}

void nv_free_buffers(struct nv_editor* editor) {
    NV_ASSERT(editor->buffers);
    for (size_t i = 0; i < vector_size(editor->buffers); i++) {
        if (editor->buffers[i].file != NULL)
            fclose(editor->buffers[i].file);
        vector_free(editor->buffers[i].lines);
        vector_free(editor->buffers[i].buffer);
        editor->buffers[i].buffer = NULL;
    }
    vector_free(editor->buffers);
    editor->buffers = NULL;
}
