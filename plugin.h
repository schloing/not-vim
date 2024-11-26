#ifndef NV_PLUGIN_H
#define NV_PLUGIN_H

#include "events.h"

struct nv_plugin {
    const char* name;
    const char* author;
    const int iteration;
    enum nv_event_sub subs;
    void (*main)(void);
};

#endif
