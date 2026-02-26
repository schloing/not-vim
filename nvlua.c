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

#include "nvapi.h"
#include "nvlua.h"

#define NV_PLUGIN_ENTRYPOINT "main.lua"
#define NV_PLUGIN_ENTRYPOINT_LENGTH (sizeof(NV_PLUGIN_ENTRYPOINT) - 1)

static int nvlua_main();
static void nvlua_free();
static int nv_load_plugin(lua_State* L, char* path);
static int nv_open_plugdir(lua_State* L, char* path, struct stat* sb);
static void nvlua_register_es(lua_State* L);

static int nv_log_lua(lua_State* L);                // for nvapi->nv_log
static int nvlua_subscribe_event(lua_State* L);     // for nv_event_register_sub

static const struct nv_api* nvapi;
static lua_State* L; // global lua state

static int nv_log_lua(lua_State* L)
{
    if (!L) {
        return NV_ERR;
    }

    const char* str = luaL_checkstring(L, 1);
    nvapi->nv_log(str);
    return NV_OK;
}

static int nv_open_plugdir(lua_State* L, char* path, struct stat* sb)
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
            nvapi->nv_log("%s loaded succesfully\n", path);
        } else {
            const char* strerr = lua_tostring(L, -1);
            nvapi->nv_log(strerr);
        }
    }

    free(plugin_entry_path);
    return NV_OK;
}

static int nv_load_plugin(lua_State* L, char* path)
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
        (void)nv_open_plugdir(L, path, &sb);
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
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    nvapi->nv_event_register_sub(event, ref);
    nvapi->nv_log("subscribed to event %s\n", nvapi->nv_event_str(event));

    return NV_OK;
}

static void nvlua_register_es(lua_State* L)
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

static struct nvlua_api nvlua_api;

const struct nvlua_api* nvlua_plugin_init(const struct nv_api* api)
{
    if (!api) {
        // trust that caller closes plugin
        return NULL;
    }
    nvapi = api;
    nvlua_api = (struct nvlua_api) {
        .nvlua_main = nvlua_main,
        .nvlua_free = nvlua_free,
    };
    return &nvlua_api;
}

static int nvlua_main()
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

    nvlua_register_es(L);

    lua_register(L, "echo", nv_log_lua);

    nv_load_plugin(L, "./plugload/");

    return NV_OK;
}

static void nvlua_free()
{
    if (L) {
        lua_close(L);
    }
}
#endif
