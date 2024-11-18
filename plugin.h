#ifndef NV_PLUGIN_H
#define NV_PLUGIN_H

struct nv_plugin {
    const char* name;
    const char* author;
    const int iteration;
    void (*main)(void);
};

#endif
