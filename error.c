#include "error.h"

const char* nv_strerror(int code) {
    switch (code) {
        case NV_OK:             return "NV_OK: No error";
        case NV_ERR:            return "NV_ERR: Generic error";
        case NV_ERR_DEP:        return "NV_ERR_DEP: Dependency error";
        case NV_ERR_NOT_INIT:   return "NV_ERR_NOT_INIT: Not initialized";
        case NV_ERR_MEM:        return "NV_ERR_MEM: Memory allocation error";
        default:                return "Unknown error code";
    }
}
