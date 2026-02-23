// events are built for lua plugins only
// dynamic .so are not gunna be supported for now, so not gunna generalise events
#ifndef NV_NO_LUAJIT
#include <string.h>

#include "cvector.h"
#include "events.h"

#define NV_LUA_DEFAULT_CALLBACK_CAP 32
static cvector(int) event_lua_callbacks[NV_EVENT_COUNT]; // each [NV_EVENT] can map to multiple luaL_ref values

void nv_event_init()
{
    for (int i = 0; i < NV_EVENT_COUNT; i++) {
        cvector_reserve(event_lua_callbacks[i], NV_LUA_DEFAULT_CALLBACK_CAP);
    }
}

void nv_event_free()
{
    for (int i = 0; i < NV_EVENT_COUNT; i++) {
        if (!event_lua_callbacks[i]) {
            continue;
        }
        cvector_free(event_lua_callbacks[i]);
    }
}

// does NOT type check lua_callback_ref, caller is expected to do that from lua context
void nv_event_register_sub(enum nv_event_sub event, int lua_callback_ref)
{
    if (event < 0 || event >= NV_EVENT_COUNT) {
        return;
    }

    if (event_lua_callbacks[(int)event]) {
        cvector_push_back(event_lua_callbacks[(int)event], lua_callback_ref);
    }
}

const char* nv_event_str(enum nv_event_sub event)
{
    switch (event) {
    case NV_EVENT_BUFFLOAD:         return "BUFFLOAD";
    case NV_EVENT_BUFFCLOSE:        return "BUFFCLOSE";
    case NV_EVENT_KEYPRESS:         return "KEYPRESS";
    case NV_EVENT_HOTKEY:           return "HOTKEY";
    default:                        return "UNKNOWN";
    }
}

enum nv_event_sub nv_str_event(const char* str)
{
    for (enum nv_event_sub event = 0; event < NV_EVENT_COUNT; event++) {
        if (strcmp(str, nv_event_str(event)) == 0) {
            return event;
        }
    }

    return NV_EVENT_COUNT;
}
#endif