#ifndef NV_LUA_H
#define NV_LUA_H

#ifdef NV_LUAJIT

#include <lua.h>
#include "window.h"

int nvlua_main();
int l_dir(lua_State* L);
void nvlua_free();
void nvlua_pcall(int lua_func_ref, struct nv_context* ctx);

struct lua_func_sig {
    int nargs;
    int nresults;
    int errfunc;
};

int nvlua_push_nv_context(struct nv_context* ctx);

#endif

#endif
