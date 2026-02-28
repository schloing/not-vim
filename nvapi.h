#ifndef NV_API_H
#define NV_API_H

#include <stddef.h>
#include "events.h"

struct nv_api {
    void (*nv_log)(const char* fmt, ...);
    void (*nv_event_register_sub)(enum nv_event_sub event, int lua_callback_ref);
    const char* (*nv_event_str)(enum nv_event_sub event);
    enum nv_event_sub (*nv_str_event)(const char* string);
    void* reserved[16];
};

struct nvlua_api {
    int (*nvlua_main)();
    void (*nvlua_free)();
    void* reserved[16];
};

struct nvrpc_api {
    int (*nvrpc_main)();
    void (*nvrpc_free)();
    void* reserved[16];
    // TODO:
};

struct nv_api_version {
    size_t min;
    size_t max;
};

typedef const void* (*nv_plugin_init_t)(size_t indicated_version, const struct nv_api* api);

#endif