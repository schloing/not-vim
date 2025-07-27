#ifndef NV_LUA_H
#define NV_LUA_H

#include "editor.h"

int nvlua_main();

#include <luajit-2.1/lua.h>

int l_dir(lua_State* L);

#endif
