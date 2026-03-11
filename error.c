#include "error.h"

const char* nv_strerror(nv_err code) {
    switch (code) {
        case NV_OK:                 return "NV_OK: No error";
        case NV_ERR:                return "NV_ERR: Generic error";
        case NV_ERR_DEP:            return "NV_ERR_DEP: Dependency error";
        case NV_ERR_NOT_INIT:       return "NV_ERR_NOT_INIT: Not initialized";
        case NV_ERR_MEM:            return "NV_ERR_MEM: Memory allocation error";
        case NV_WARN:               return "NV_WARN: Generic warn";
        case NV_WARN_UNIMPLEMENTED: return "NV_WARN_UNIMPLEMENTED: Unimplemented";
        default:                    return "Unknown error code";
    }
}
