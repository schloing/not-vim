#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "assert.h"
#include "buffer.h"
#include "editor.h"
#include "vec.h"

struct nv_buff* nv_buffer_init(char* path) {
    NV_ASSERT(path);
    struct nv_buff* buff = (struct nv_buff*)malloc(sizeof(struct nv_buff));
    NV_ASSERT(buff);
#define NV_BUFFID_UNSET 0
    buff->id = NV_BUFFID_UNSET;
    buff->path = path;
    buff->file = fopen(path, "r+");
   
    struct stat sb;
    fstat(fileno(buff->file), &sb);
    
    switch (sb.st_mode & S_IFMT) {
    case S_IFLNK: // symlink
    case S_IFDIR:
        buff->type = NV_BUFFTYPE_BROWSER;
        break;
    
    case S_IFREG:
        buff->type = NV_BUFFTYPE_SOURCE;
        break;
    
    case S_IFSOCK:
        buff->type = NV_BUFFTYPE_NETWORK;
        break;
    
    default:
        free(buff);
        return NULL;
    }

    buff->buffer = vector_create();
//  buff->buffer = malloc(sb.st_size + 4096);

    return buff;
}

