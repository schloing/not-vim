#ifndef NOT_VIM_H
#define NOT_VIM_H

#include <stdlib.h>
#include <stdbool.h>

#include "vec.h"

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

enum {
    NV_BUFFTYPE_STDIN    = 1,
    NV_BUFFTYPE_STDOUT   = 2,
    NV_BUFFTYPE_BROWSER  = 4,
    NV_BUFFTYPE_NETWORK  = 8,
    NV_BUFFTYPE_SOURCE   = 16,
};

typedef int nv_buff_t;
typedef char* char_vec; // TODO: implement vec.h

struct nv_buff {
    size_t      id;       // id for buffer
    nv_buff_t   type;     // what the buffer shows
    char*       path;     // path of buffer
    FILE*       file;     // FILE* if applicable
    char_vec    buffer;   // char buffer in memory written to file on write
    struct {              // position
        int     line;
        int     col;
    };
};

struct nv_buff* nv_buffer_init(char* path);

#endif
