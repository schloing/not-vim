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
    if (!buff || !path || !buff->buffer) {
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

        break;

    case S_IFSOCK:
        buff->type = NV_BUFFTYPE_NETWORK;
        break;

    default:
        return NV_ERR;
    }

    return NV_OK;
}

struct nv_view* nv_view_init(const char* buffer_file_path)
{
    struct nv_view* view = (struct nv_view*)calloc(1, sizeof(struct nv_view));

    if (!view) {
        nv_editor->status = NV_ERR_MEM;
        return NULL;
    }

    view->top_line_index = 0;
    view->buffer = nv_buffer_init(buffer_file_path);

    static_assert(NV_CURSOR_CAP > NV_PRIMARY_CURSOR, "");
    cvector_reserve(view->cursors, NV_CURSOR_CAP);
    view->cursors[NV_PRIMARY_CURSOR] = (struct cursor) { 0 };
    
    if (nv_editor->status != NV_OK) {
        return NULL;
    }

    cvector_reserve(view->map, (size_t)NV_MAP_CAP);

    return view;
}

struct nv_buff* nv_buffer_init(const char* path)
{
    struct nv_buff* buffer = (struct nv_buff*)calloc(1, sizeof(struct nv_buff));

    if (!buffer) {
        nv_editor->status = NV_ERR_NOT_INIT;
        return NULL;
    }

    buffer->chunk = NV_BUFF_CHUNK_SIZE;
    cvector_reserve(buffer->buffer, (size_t)NV_BUFF_CHUNK_SIZE);
    cvector_reserve(buffer->lines, (size_t)NV_LINE_CAP);

    if (path) {
        buffer->path = (char*)path;
        nv_editor->status = nv_buffer_open_file(buffer, path);
        if (nv_editor->status != NV_OK) {
            return NULL;
        }
    }

    return buffer;
}

struct nv_buff_line* line(struct nv_context* ctx, size_t lineno)
{
    if (ctx->view->line_count < lineno) {
        return NULL;
    }

    return &ctx->buffer->lines[lineno];
}

int nv_rebuild_lines(struct nv_buff* buff, int* out_line_count)
{
    char* b = buff->buffer;

    if (!b) {
        return NV_ERR_NOT_INIT;
    }

    struct nv_buff_line line = { 0 };
    size_t i = 0;
    int line_count = 0;

    while (b[i] != '\0') {
        if (b[i] == '\n') {
            line.end = i;
            line.length = line.end - line.begin;

            cvector_push_back(buff->lines, line);

            line.begin = i + 1;
            line_count++;
        }
        i++;
    }

    if (i > line.begin) {
        line.end = i;
        line.length = line.end - line.begin;
        cvector_push_back(buff->lines, line);
        line_count++;
    }

    // empty file
    if (line_count == 0) {
        line.begin = 0;
        line.end = 0;
        line_count = 1;
    }

    *out_line_count = line_count;
    return NV_OK;
}

int nv_free_view(struct nv_view* view)
{
    if (!view) {
        return NV_ERR_NOT_INIT;
    }

    cvector_free(view->visual_rows);
    cvector_free(view->cursors);
    cvector_free(view->map);
    nv_free_buffer(view->buffer);

    free(view);
    view = NULL;
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

    cvector_free(buff->lines);
    cvector_free(buff->buffer);

    free(buff);
    buff = NULL;
    return NV_OK;
}
