#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "buffer.h"
#include "cvector.h"
#include "editor.h"
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

void nv_buffer_open_file(struct nv_buff* buff, const char* path)
{
    if (!buff || !path) {
        return;
    }
    if (!buff->cursors || !buff->buffer) {
        return;
    }

    struct stat sb;
    if (stat(buff->path, &sb) == -1) {
        return;
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
            return;
        }

        fread(buff->buffer, sizeof(char), buff->chunk, buff->file);
        cvector_set_size(buff->buffer, buff->chunk);

        buff->cursors[0].ch = buff->buffer[0];
        break;

    case S_IFSOCK:
        buff->type = NV_BUFFTYPE_NETWORK;
        break;

    default:
        return;
    }
}

void nv_buffer_init(struct nv_buff* buff, char* path)
{
    if (!buff) {
        return;
    }
    cvector_reserve(buff->cursors, NV_CURSOR_CAP);
    assert(NV_CURSOR_CAP > NV_PRIMARY_CURSOR);
    buff->cursors[NV_PRIMARY_CURSOR] = (struct cursor) { 0 };
    cvector_reserve(buff->lines, NV_LINE_CAP);
    cvector_reserve(buff->buffer, NV_BUFF_CAP);

    buff->chunk = NV_BUFF_CAP;
    buff->path = path;
    buff->top_line = 0;

    nv_buffer_open_file(buff, path);
}

struct nv_buff_line* currline(struct nv_buff* buff)
{
    size_t line = buff->cursors[NV_PRIMARY_CURSOR].line;
    return &buff->lines[line];
}

struct nv_buff_line* prevline(struct nv_buff* buff)
{
    size_t line = buff->cursors[NV_PRIMARY_CURSOR].line;
    if ((int)line <= 0) {
        return NULL;
    }
    return &buff->lines[line - 1];
}

struct nv_buff_line* nextline(struct nv_buff* buff)
{
    size_t line = buff->cursors[NV_PRIMARY_CURSOR].line;
    if (buff->line_count < line + 1) {
        return NULL;
    }
    return &buff->lines[line + 1];
}

struct nv_buff_line* line(struct nv_buff* buff, size_t lineno)
{
    if (buff->line_count < lineno) {
        return NULL;
    }
    return &buff->lines[lineno];
}

void nv_load_file_buffer(struct nv_buff* buff, size_t* out_line_count)
{
    char* b = buff->buffer;
    if (!b) {
        return;
    }
    struct nv_buff_line line = { 0 };
    size_t i = 0;
    size_t line_count = 0;

    while (b[i++] != '\0') {
        if (b[i] == '\n') {
            line.end = i;
            line.length = line.end - line.begin;

            cvector_push_back(buff->lines, line);

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

void nv_free_buffer(struct nv_buff* buff)
{
    if (!buff) {
        return;
    }

    if (buff->file) {
        fclose(buff->file);
    }

    cvector_free(buff->cursors);
    cvector_free(buff->lines);
    cvector_free(buff->buffer);
    free(buff);
    buff = NULL;
}
