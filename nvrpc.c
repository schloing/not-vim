#include <nng/nng.h>
#include <nng/protocol/reqrep0/req.h>
#include <stddef.h>

#include "error.h"
#include "nvapi.h"
#include "nvrpc.h"

// plugin registration, nvapi compat
static const struct nv_api* nvapi;
static struct nvrpc_api nvrpc_api;
static const struct nv_api_version version = {
    .min = 0,
    .max = 0,
};
// end plugin

static struct nvrpc_state nvrpc = { 0 };

nv_plugin_init_t nvrpc_plugin_init(size_t indicated_version, const struct nv_api* api)
{
    if (!api) {
        return NULL;
    }

    nvapi = api;

    nvrpc_api = (struct nvrpc_api) {
        .nvrpc_main = nvrpc_main,
        .nvrpc_free = nvrpc_free,
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

int nvrpc_main()
{
    nng_socket sock;
    nng_listener listener;
    size_t status;

    if ((status = nng_req_open(&sock)) != 0) {
        nvapi->nv_log("nvrpc failed to open socket: %s\n", nng_strerror(status));
        return NV_ERR;
    }

    if ((status = nng_listener_create(&listener, sock, NVRPC_LISTEN_URL)) != 0) {
        nvapi->nv_log("nvrpc failed to create listener: %s\n", nng_strerror(status));
        nng_socket_close(sock);
        return NV_ERR;
    }

    nng_socket_set_ms(sock, NNG_OPT_REQ_RESENDTIME, 2000);
	nng_listener_start(listener, 0);

    nvapi->nv_log("succesfully opened rpc socket\n");
    nvrpc.s = sock;
    nvrpc.l = listener;
    return NV_OK;
}

void nvrpc_free()
{
    nng_listener_close(nvrpc.l);
    // nng_socket_close(nvrpc.s);
    return;
}