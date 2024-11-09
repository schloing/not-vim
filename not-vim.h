#ifndef NOT_VIM_H
#define NOT_VIM_H

enum {
    NV_BUFFTYPE_STDIN    = 1,
    NV_BUFFTYPE_STDOUT   = 2,
    NV_BUFFTYPE_BROWSER  = 4,
    NV_BUFFTYPE_NETWORK  = 8,
    NV_BUFFTYPE_SOURCE   = 16,
};

typedef int nv_buff_t;

struct nv_buff {
    size_t      id;
    char*       name;
    nv_buff_t   type;
    FILE*       file;
};

#endif
