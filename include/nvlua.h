#ifndef NV_LUA_H
#define NV_LUA_H

#ifdef NV_LUAJIT

#include <lua.h>

int l_dir(lua_State* L);

struct lua_func_sig {
    int nargs;
    int nresults;
    int errfunc;
};

#endif

#endif
