#ifndef NV_EVENTS_H
#define NV_EVENTS_H

enum nv_event_sub {
    NV_EVENT_BUFFLOAD,
    NV_EVENT_BUFFCLOSE,
    NV_EVENT_KEYPRESS,
    NV_EVENT_HOTKEY,
    NV_EVENT_COUNT,
};

void nv_register_event(enum nv_event_sub event, const char* name, void (*callback)(void));

#endif
