#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "buffer.h"
#include "cvector.h"
#include "editor.h"
#include "error.h"
#include "termbox2.h"

bool is_elf(const char* buffer)
{
    const char e_ident[] = { 0x7f, 45, 0x4c, 46 };
    for (int i = 0; i < 4; i++) {
        if (e_ident[i] != buffer[i]) {
            return false;
        }
    }
    return true;
}

int nv_buffer_open_file(struct nv_buff* buff, const char* path)
{
    if (!buff || !path || !buff->cursors || !buff->buffer) {
        return NV_ERR_NOT_INIT;
    }

    struct stat sb;
    if (stat(buff->path, &sb) == -1) {
        return NV_ERR;
    }

    switch (sb.st_mode & S_IFMT) {
    case S_IFLNK: // symlink
    case S_IFDIR:
        buff->type = NV_BUFFTYPE_BROWSER;
        break;

    case S_IFREG:
        buff->type = NV_BUFFTYPE_SOURCE;
        buff->file = fopen(buff->path, "rb+");

        if (buff->file == NULL) {
            return NV_ERR;
        }

        fread(buff->buffer, sizeof(char), buff->chunk, buff->file);
        cvector_set_size(buff->buffer, buff->chunk);

        buff->cursors[0].ch = buff->buffer[0];
        break;

    case S_IFSOCK:
        buff->type = NV_BUFFTYPE_NETWORK;
        break;

    default:
        return NV_ERR;
    }

    return NV_OK;
}

struct nv_buff* nv_buffer_init(const char* path)
{
    struct nv_buff* buffer = (struct nv_buff*)calloc(1, sizeof(struct nv_buff));

    if (!buffer) {
        nv_editor->status = NV_ERR_NOT_INIT;
        nv_fatal("error initialising buffer");
        // FIXME: really bad error
        // return NV_ERR_NOT_INIT;
        return NULL;
    }

    cvector_reserve(buffer->cursors, NV_CURSOR_CAP);
    static_assert(NV_CURSOR_CAP > NV_PRIMARY_CURSOR, "");
    buffer->cursors[NV_PRIMARY_CURSOR] = (struct cursor) { 0 };
    cvector_reserve(buffer->lines, (size_t)NV_LINE_CAP);
    cvector_reserve(buffer->buffer, (size_t)NV_BUFF_CAP);

    buffer->chunk = NV_BUFF_CAP;
    buffer->top_line = 0;

    if (path) {
        buffer->path = (char*)path;
        (void)nv_buffer_open_file(buffer, path);
    }

    return buffer;
}

struct nv_buff_line* line(struct nv_buff* buff, size_t lineno)
{
    if (buff->line_count < lineno) {
        return NULL;
    }
    return &buff->lines[lineno];
}

int nv_load_file_buffer(struct nv_buff* buff, size_t* out_line_count)
{
    char* b = buff->buffer;

    if (!b) {
        return NV_ERR_NOT_INIT;
    }

    struct nv_buff_line line = { 0 };
    size_t i = 0;
    size_t line_count = 0;

    while (b[i++] != '\0') {
        if (b[i] == '\n') {
            line.end = i;
            line.length = line.end - line.begin - (line.end == line.begin ? 0 : 1); // subtract the new line if the line isn't empty

            cvector_push_back(buff->lines, line);

            line.begin = i + 1;
            line_count++;
        }
    }

    line_count--;

    // empty file
    if (line_count == 0) {
        line.begin = 0;
        line.end = 0;
        line_count = 1;
    }

    *out_line_count = line_count;
    return NV_OK;
}

int nv_free_buffer(struct nv_buff* buff)
{
    if (!buff) {
        return NV_ERR_NOT_INIT;
    }

    if (buff->file) {
        (void)fclose(buff->file);
        buff->file = NULL;
    }

    cvector_free(buff->cursors);
    cvector_free(buff->lines);
    cvector_free(buff->buffer);
    free(buff);
    buff = NULL;
    return NV_OK;
}
