#include <nng.h>
#include <stddef.h>

#include "nvapi.h"
#include "rpc.h"

static const struct nv_api* nvapi;
static struct nvrpc_api nvrpc_api;
static const struct nv_api_version version = {
    .min = 0,
    .max = 0,
};

nv_plugin_init_t nvrpc_plugin_init(size_t indicated_version, const struct nv_api* api)
{
    if (!api) {
        return NULL;
    }

    nvapi = api;

    nvrpc_api = (struct nvrpc_api) {
        // TODO
    };

    if (indicated_version > version.max) {
        nvapi->nv_log("nvlua indicated nvapi version %zu is greater than max supported nvapi version %zu\n", indicated_version, version.max);
        nvapi = NULL;
        return NULL;
    }

    if (indicated_version < version.min) {
        nvapi->nv_log("nvlua indicated nvapi version %zu is less than min supported nvapi version %zu\n", indicated_version, version.min);
        nvapi = NULL;
        return NULL;
    }

    return (void*)&nvrpc_api;
}