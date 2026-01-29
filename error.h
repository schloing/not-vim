#ifndef NV_ERROR_H
#define NV_ERROR_H

typedef enum {
    NV_ERR             = -128,
    NV_ERR_DEP,
    NV_ERR_NOT_INIT,
    NV_ERR_MEM,
    NV_OK              = 0,
    NV_WARN,
} nv_err;

const char* nv_strerror(nv_err code);

#endif
