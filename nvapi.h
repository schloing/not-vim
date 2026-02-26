#ifndef NV_API_H
#define NV_API_H

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

typedef const struct nvlua_api* (*nvlua_plugin_init_t)(const struct nv_api* api);

#endif