#ifndef NV_RPC_H
#define NV_RPC_H

#include <nng/nng.h>

struct nvrpc_state {
    nng_socket s;
    nng_listener l;
};

int nvrpc_main();
void nvrpc_free();

#define __NVRPC_SOCK_NAME "nv-rpc"

#ifdef __linux__
#define NVRPC_LISTEN_URL "ipc:///tmp/" __NVRPC_SOCK_NAME ".ipc"
#elif defined(__APPLE__)
#define NVRPC_LISTEN_URL "ipc:///run/user/1000/" __NVRPC_SOCK_NAME ".ipc"
#endif

#endif