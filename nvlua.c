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

#include "editor.h"
#include "error.h"
#include "events.h"
#include "nvlua.h"

#define NV_PLUGIN_ENTRYPOINT "main.lua"
#define NV_PLUGIN_ENTRYPOINT_LENGTH (sizeof(NV_PLUGIN_ENTRYPOINT) - 1)

// forwards

static int nv_load_plugin(lua_State* L, char* path);
static int nv_open_plugdir(lua_State* L, char* path, struct stat* sb);
static void nvlua_set_metatable(lua_State* L, const char* name);
static void nvlua_register_es(lua_State* L);
static void nvlua_register_nv_api(lua_State* L);

// lua wrappers
static int nv_log_lua(lua_State* L);                // for nv_log
static int nvlua_subscribe_event(lua_State* L);     // for nv_event_register_sub
static int nvctx_get_buffer(lua_State* L);
static int nvlua_context_gc(lua_State* L);

// api
static struct nv_context* nvlua_check_nv_context(lua_State* L, int idx);
static struct nv_buff* nvlua_check_nv_buff(lua_State* L, int idx);
// nv_buff getters
static int nvbuff_get_path(lua_State* L);
static int nvbuff_get_line_count(lua_State* L);
static int nvbuff_get_bytes_loaded(lua_State* L);
static int nvbuff_get_buffer_size(lua_State* L);

// end forwards

lua_State* L; // global lua state

static int nv_log_lua(lua_State* L)
{
    if (!L) {
        return NV_ERR;
    }

    const char* str = luaL_checkstring(L, 1);
    nv_log(str);
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
            nv_log("%s loaded succesfully\n", path);
        } else {
            const char* strerr = lua_tostring(L, -1);
            nv_log(strerr);
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

static int nvlua_subscribe_event(lua_State* L)
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
    nv_event_register_sub(event, ref);

    nv_log("subscribed to event %s\n", nv_event_str(event));
    return NV_OK;
}

static void nvlua_set_metatable(lua_State* L, const char* name)
{
    luaL_getmetatable(L, name);
    lua_setmetatable(L, -2); // userdata remains on stack
}

static struct nv_context* nvlua_check_nv_context(lua_State* L, int idx)
{
    struct nv_context** uc = (struct nv_context**)luaL_checkudata(L, idx, "nv_context");
    if (!uc || !*uc) {
        luaL_error(L, "invalid nv_context");
    }
    return *uc;
}

int nvlua_push_nv_context(struct nv_context* ctx)
{
    struct nv_context** uc = (struct nv_context**)lua_newuserdata(L, sizeof(struct nv_context*));
    *uc = ctx;
    nvlua_set_metatable(L, "nv_context");
    return 1;
}

int nvlua_push_nv_buff(struct nv_buff* b)
{
    struct nv_buff** ub = (struct nv_buff**)lua_newuserdata(L, sizeof(struct nv_buff*));
    *ub = b;
    nvlua_set_metatable(L, "nv_buff");
    return 1;
}

static int nvctx_get_buffer(lua_State* L)
{
    struct nv_context* ctx = nvlua_check_nv_context(L, 1);
    if (!ctx->buffer) {
        lua_pushnil(L);
        return 1;
    }
    return nvlua_push_nv_buff(ctx->buffer);
}

static int nvlua_context_gc(lua_State* L)
{
    struct nv_context** uc = (struct nv_context**)luaL_checkudata(L, 1, "nv_context");
    *uc = NULL; // clear pointer so gc can't free it
    return NV_OK;
}

static struct nv_buff* nvlua_check_nv_buff(lua_State* L, int idx)
{
    struct nv_buff** ub = (struct nv_buff**)luaL_checkudata(L, idx, "nv_buff");
    if (!ub || !*ub) {
        luaL_error(L, "invalid nv_buff");
    }
    return *ub;
}

static int nvbuff_get_path(lua_State* L)
{
    struct nv_buff* b = nvlua_check_nv_buff(L, 1);
    if (b->path) {
        lua_pushstring(L, b->path);
    }
    else {
        lua_pushnil(L);
    }
    return 1;
}

static int nvbuff_get_line_count(lua_State* L)
{
    struct nv_buff* b = nvlua_check_nv_buff(L, 1);
    lua_pushinteger(L, b->line_count);
    return 1;
}

static int nvbuff_get_bytes_loaded(lua_State* L)
{
    struct nv_buff* b = nvlua_check_nv_buff(L, 1);
    lua_pushinteger(L, (lua_Integer)b->bytes_loaded);
    return 1;
}

static int nvbuff_get_buffer_size(lua_State* L)
{
    struct nv_buff* b = nvlua_check_nv_buff(L, 1);
    lua_pushinteger(L, (lua_Integer)(b->chunk_size));
    return 1;
}

static void nvlua_register_nv_api(lua_State* L)
{
    luaL_newmetatable(L, "nv_buff");

    lua_newtable(L);
    // TODO: buffer methods
    lua_pushcfunction(L, nvbuff_get_path);
    lua_setfield(L, -2, "path");
    lua_pushcfunction(L, nvbuff_get_line_count);
    lua_setfield(L, -2, "line_count");
    lua_pushcfunction(L, nvbuff_get_bytes_loaded);
    lua_setfield(L, -2, "bytes_loaded");
    lua_pushcfunction(L, nvbuff_get_buffer_size);
    lua_setfield(L, -2, "buffer_size");
    lua_setfield(L, -2, "__index");

    // TODO: __gc
    lua_pop(L, 1);                              // end nv_buff metatable

    luaL_newmetatable(L, "nv_context");
    lua_newtable(L);
    // TODO: context methods
    lua_pushcfunction(L, nvctx_get_buffer);
    lua_setfield(L, -2, "get_buffer");   // ctx:get_buffer()
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, nvlua_context_gc);
    lua_setfield(L, -2, "__gc");

    lua_pop(L, 1);                              // end nv_context metatable

    nv_log("registered nv_context\n");

    lua_setglobal(L, "nv");

    nv_log("registered nvlua api structures\n");
}

static void nvlua_register_es(lua_State* L)
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

void nvlua_pcall(int lua_func_ref, struct nv_context* ctx)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, lua_func_ref);
    nvlua_push_nv_context(ctx);
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        const char* strerr = lua_tostring(L, -1);
        nv_log("%s\n", strerr);
    }
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

    nvlua_register_nv_api(L);
    nvlua_register_es(L);

    lua_register(L, "echo", nv_log_lua);

    nv_load_plugin(L, "./plugload/");

    return NV_OK;
}

void nvlua_free()
{
    if (L) {
        lua_close(L);
    }
}
#endif
