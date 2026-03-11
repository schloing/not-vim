#ifndef NV_EVENTS_H
#define NV_EVENTS_H

#include <stdbool.h>
#include "window.h"

enum nv_event_sub {
    NV_EVENT_BUFFLOAD,
    NV_EVENT_BUFFCLOSE,
    NV_EVENT_BUFFDRAW,
    NV_EVENT_WINDOW_SPLIT,
    NV_EVENT_KEYPRESS,
    NV_EVENT_HOTKEY,
    NV_EVENT_COUNT,
};

enum nv_event_type {
    NV_EVENT_TYPE_REMOTE_RPC,
    NV_EVENT_TYPE_LUA_REF,
    NV_EVENT_TYPE_COUNT,
};

struct nv_event_callback {
    enum nv_event_type type;

    union {
        // TODO: rpc event name, args, channel, etc.
        int ref;
    };
};

const char* nv_event_str(enum nv_event_sub event);
void nv_event_register_sub(enum nv_event_sub event, struct nv_event_callback callback);
enum nv_event_sub nv_str_event(const char* str);
int nv_event_emit(enum nv_event_sub event, struct nv_context* ctx);
const bool nv_event_is_valid(enum nv_event_sub event);
void nv_event_init();
void nv_event_free();

#endif
