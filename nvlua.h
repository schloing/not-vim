#ifndef NV_LUA_H
#define NV_LUA_H

#ifndef NV_NO_LUAJIT

#include <lua.h>

int nvlua_main();
int l_dir(lua_State* L);

#endif

#endif
