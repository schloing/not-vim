// https://github.com/yasuoka/luacstruct
// https://www.lua.org/pil/contents.html
#ifndef NV_NO_LUAJIT
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cvector.h"
#include "editor.h"
#include "error.h"
#include "events.h"
#include "nvlua.h"

#define NV_PLUGIN_ENTRYPOINT "main.lua"
#define NV_PLUGIN_ENTRYPOINT_LENGTH (sizeof(NV_PLUGIN_ENTRYPOINT) - 1)

// forwards
static int nv_load_plugin(char* path);
static int nv_open_plugdir(char* path, struct stat* sb);
static void nvlua_register_library();
// lua wrappers
static int nv_log_lua();                // for nv_log
static int nvlua_subscribe_event();     // for nv_event_register_sub
// end forwards

lua_State* L; // global lua state

static int nv_log_lua()
{
    if (!L) {
        return NV_ERR;
    }

    const char* str = luaL_checkstring(L, 1);
    nv_log(str);
    return NV_OK;
}

static int nv_open_plugdir(char* path, struct stat* sb)
{
    if (!path || !L) {
        return NV_ERR_NOT_INIT;
    }

    size_t path_length = strlen(path);
    // all plugdirs must contain a NV_PLUGIN_ENTRYPOINT
    char* plugin_entry_path = (char*)calloc(path_length + NV_PLUGIN_ENTRYPOINT_LENGTH + 1, sizeof(char)); // null terminator

    if (!plugin_entry_path) {
        return NV_ERR_MEM;
    }

    memcpy(plugin_entry_path, path, path_length); // base path prefix
    memcpy(plugin_entry_path + path_length, NV_PLUGIN_ENTRYPOINT, NV_PLUGIN_ENTRYPOINT_LENGTH); // entrypoint suffix

    if (stat(plugin_entry_path, sb) == -1) {
        free(plugin_entry_path);
        return NV_ERR;
    }

    if ((sb->st_mode & S_IFMT) == S_IFREG) {
        if (luaL_dofile(L, plugin_entry_path) == LUA_OK) {
            nv_log("%s loaded succesfully\n", path);
        } else {
            const char* strerr = lua_tostring(L, -1);
            nv_log(strerr);
        }
    }

    free(plugin_entry_path);
    return NV_OK;
}

static int nv_load_plugin(char* path)
{
    if (!path || !L) {
        return NV_ERR_NOT_INIT;
    }

    struct stat sb;
    if (stat(path, &sb) == -1) {
        return NV_ERR;
    }

    switch (sb.st_mode & S_IFMT) {
    case S_IFDIR:
        (void)nv_open_plugdir(path, &sb);
        break;

    default:
        return NV_ERR;
    }

    return NV_OK;
}

static int nvlua_get_event(lua_State* L, int idx) {
    if (lua_type(L, idx) == LUA_TNUMBER) {
        int id = (int)luaL_checkinteger(L, idx);

        if (id < 0 || id >= NV_EVENT_COUNT) {
            nv_log("invalid event id\n");
            luaL_argerror(L, idx, "invalid event id");
        }

        return id;
    }
    else {
        const enum nv_event_sub event = nv_str_event(luaL_checkstring(L, idx));
    
        if (event == NV_EVENT_COUNT) {
            nv_log("unknown event name\n");
            luaL_argerror(L, idx, "unknown event name");
            return NV_EVENT_COUNT;
        }

        return event;
    }
}

static int nvlua_subscribe_event()
{
    // event id / name
    enum nv_event_sub event = nvlua_get_event(L, 1);
    if (event == NV_EVENT_COUNT) {
        nv_log("failed event registration\n");
        return NV_ERR;
    }

    // callback
    luaL_checktype(L, 2, LUA_TFUNCTION);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    nv_event_register_sub(NV_EVENT_COUNT, ref);

    lua_pushinteger(L, ref);
    nv_log("subscribed to event %s\n", nv_event_str(event));
    return NV_OK;
}

static void nvlua_register_library()
{
    lua_newtable(L);

    // register nv_event_sub enum
    // event.EVENT_STR_NAME
    for (enum nv_event_sub event = 0; event < NV_EVENT_COUNT; event++) {
        lua_pushinteger(L, event);
        lua_setfield(L, -2, nv_event_str(event));
    }

    // register core api
    // event.subscribe()
    lua_pushcfunction(L, nvlua_subscribe_event);
    lua_setfield(L, -2, "subscribe");

    lua_setglobal(L, "event"); 
    nv_log("registered nvlua library\n");
}

int nvlua_main()
{
    L = luaL_newstate(); /* opens Lua */

    if (!L) {
        return NV_ERR;
    }

    luaL_openlibs(L); // FIXME: needs sandboxing, allows arbitrary loading of c modules from lua

#ifdef NVLUA_SANDBOXED
    luaopen_base(L); /* opens the basic library */
    luaopen_table(L); /* opens the table library */
    luaopen_io(L); /* opens the I/O library */
    luaopen_string(L); /* opens the string lib. */
    luaopen_math(L); /* opens the math lib. */
#endif

    nvlua_register_library();

    lua_register(L, "echo", nv_log_lua);

    nv_load_plugin("./plugload/");

    return NV_OK;
}

void nvlua_free()
{
    if (L) {
        lua_close(L);
    }
}
#endif
