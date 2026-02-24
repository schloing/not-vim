#ifndef NV_EVENTS_H
#define NV_EVENTS_H

#include <stdbool.h>
#include "nvlua.h"
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

const char* nv_event_str(enum nv_event_sub event);
void nv_event_register_sub(enum nv_event_sub event, int lua_callback_ref);
enum nv_event_sub nv_str_event(const char* str);
int nv_event_emit(enum nv_event_sub event, struct nv_context* ctx);
const bool nv_event_is_valid(enum nv_event_sub event);
void nv_event_init();
void nv_event_free();

extern const struct lua_func_sig lua_callback_func_sigs[NV_EVENT_COUNT];

#endif
