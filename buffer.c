#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "assert.h"
#include "buffer.h"
#include "cvector.h"
#include "editor.h"
#include "termbox2.h"

#define NV_BUFFID_UNSET 0
#define NV_BUFF_CAP     1024 * 16
#define NV_LINE_CAP     32

bool is_elf(char* buffer) {
    const char e_ident[] = { 0x7f, 45, 0x4c, 46 };
    for (int i = 0; i < 4; i++)
        if (e_ident[i] != buffer[i]) return false;
    return true;
}

void nv_buffer_init(struct nv_buff* buff, char* path) {
    NV_ASSERT(buff);

    cvector_reserve(buff->buffer, NV_BUFF_CAP);
    cvector_push_back(buff->cursors, (struct cursor) { 0 });
    buff->chunk = cvector_capacity(buff->buffer); // should be NV_BUFF_CAP
    cvector_reserve(buff->lines, NV_LINE_CAP);
    buff->_begin_line = 0;
    
    if (path == NULL) return;
    buff->path = path;

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

        fread(cvector_begin(buff->buffer), sizeof(char), buff->chunk, buff->file);
        buff->cursors[0].ch = buff->buffer[0];
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
    if (buffer->buffer == NULL) return;
    struct nv_buff_line line = { 0 };
    int i = 0, line_count = 0;

    while (b[i++] != '\0') {
        if (b[i] == '\n') {
            line.end = i;

            cvector_push_back(buffer->lines, line);

            line.begin = i + 1;
            line_count++;
        }
    }
   
    // empty file
    if (line_count == 0) {
        line.begin = 0;
        line.end = 0;
        line_count = 1;
    }

    *out_line_count = line_count;
}

void nv_free_buffers(struct nv_editor* editor) {
    NV_ASSERT(editor->buffers);

    for (size_t i = 0; i < cvector_size(editor->buffers); i++) {
        if (editor->buffers[i].file != NULL)
            fclose(editor->buffers[i].file);
   
        cvector_free(editor->buffers[i].lines);
        cvector_free(editor->buffers[i].cursors);
        cvector_free(editor->buffers[i].buffer);
        editor->buffers[i].buffer = NULL;
    }
    
    cvector_free(editor->buffers);
    editor->buffers = NULL;
}
