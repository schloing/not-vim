#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "events.h"
#include "nvapi.h"
#include "nvlua_int.h"

#define NV_PLUGIN_ENTRYPOINT "main.lua"
#define NV_PLUGIN_ENTRYPOINT_LENGTH (sizeof(NV_PLUGIN_ENTRYPOINT) - 1)

static int nvlua_main();
static void nvlua_free();
static int nv_load_plugin(lua_State* L, char* path);
static int nv_open_plugdir(lua_State* L, char* path, struct stat* sb);
static int nv_log_lua(lua_State* L);                // for nvapi->nv_log
static int nvlua_pcall(int ref);

const struct nv_api* nvapi = NULL; // extern'd ./nvlua_int.h
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

static struct nvlua_api nvlua_api;
static const struct nv_api_version version = {
    .min = 0,
    .max = 0,
};

static int nvlua_pcall(int lua_func_ref)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, lua_func_ref);
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* strerr = lua_tostring(L, -1);
        nvapi->nv_log("%s\n", strerr);
    }
    return NV_OK;
}

nv_plugin_init_t nvlua_plugin_init(size_t indicated_version, const struct nv_api* api)
{
    if (!api) {
        // trust that caller closes plugin
        return NULL;
    }

    nvapi = api;

    nvlua_api = (struct nvlua_api) {
        .nvlua_main = nvlua_main,
        .nvlua_free = nvlua_free,
        .nvlua_pcall = nvlua_pcall,
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

    return (void*)&nvlua_api;
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