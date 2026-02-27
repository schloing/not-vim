#include <nng.h>

#include "nvapi.h"
#include "rpc.h"

static const struct nv_api* nv_api;
static struct nvrpc_api nvrpc_api;

const struct nvrpc_api* nvrpc_plugin_init(const struct nv_api* api)
{
    if (!api) {
        return NULL;
    }
    nv_api = api;
    return &nvrpc_api;
}