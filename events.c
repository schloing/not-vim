#include "events.h"

void nv_register_event(enum nv_event_sub event, const char* name, void (*callback)(void)) {
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