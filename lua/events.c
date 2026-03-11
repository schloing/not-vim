#include "events.h"
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nvapi.h"
#include "nvlua_int.h"

static int nvlua_get_event(lua_State* L, int idx);
static int nvlua_subscribe_event(lua_State* L);     // for nv_event_register_sub

static int nvlua_get_event(lua_State* L, int idx) {
    if (lua_type(L, idx) == LUA_TNUMBER) {
        int id = (int)luaL_checkinteger(L, idx);

        if (id < 0 || id >= NV_EVENT_COUNT) {
            nvapi->nv_log("invalid event id\n");
            luaL_argerror(L, idx, "invalid event id");
        }

        return id;
    }
    else {
        const enum nv_event_sub event = nvapi->nv_str_event(luaL_checkstring(L, idx));
    
        if (event == NV_EVENT_COUNT) {
            nvapi->nv_log("unknown event name\n");
            luaL_argerror(L, idx, "unknown event name");
            return NV_EVENT_COUNT;
        }

        return event;
    }
}

static int nvlua_subscribe_event(lua_State* L)
{
    // event id / name
    enum nv_event_sub event = nvlua_get_event(L, 1);
    if (event == NV_EVENT_COUNT) {
        nvapi->nv_log("failed event registration\n");
        return NV_ERR;
    }

    // callback
    luaL_checktype(L, 2, LUA_TFUNCTION);
    struct nv_event_callback callback = {
        .type=NV_EVENT_TYPE_LUA_REF,
        .ref = luaL_ref(L, LUA_REGISTRYINDEX)
    };
    nvapi->nv_event_register_sub(event, callback);
    nvapi->nv_log("subscribed to event %s\n", nvapi->nv_event_str(event));

    return NV_OK;
}

void nvlua_register_es(lua_State* L)
{
    lua_newtable(L);

    // register nv_event_sub enum
    // event.EVENT_STR_NAME
    for (enum nv_event_sub event = 0; event < NV_EVENT_COUNT; event++) {
        lua_pushinteger(L, event);
        lua_setfield(L, -2, nvapi->nv_event_str(event));
    }

    // register core api
    // event.subscribe()
    lua_pushcfunction(L, nvlua_subscribe_event);
    lua_setfield(L, -2, "subscribe");

    lua_setglobal(L, "event"); 
    nvapi->nv_log("registered nvlua library\n");
}
