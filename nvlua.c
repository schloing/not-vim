// https://github.com/yasuoka/luacstruct
// https://www.lua.org/pil/contents.html
#ifdef NV_LUAJIT
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
#include "nvlua.h"

#define NV_PLUGIN_ENTRYPOINT "main.lua"
#define NV_PLUGIN_ENTRYPOINT_LENGTH (sizeof(NV_PLUGIN_ENTRYPOINT) - 1)

static int nv_load_plugin(lua_State* L, char* path);

static int nv_log_lua(lua_State* L)
{
    const char* str = luaL_checkstring(L, 1);
    nv_log(str);
    return NV_OK;
}

static int nv_load_plugin(lua_State* L, char* path)
{
    if (!path) {
        return NV_ERR_NOT_INIT;
    }

    struct stat sb;
    if (stat(path, &sb) == -1) {
        return NV_ERR;
    }

    switch (sb.st_mode & S_IFMT) {
    case S_IFDIR: {
        size_t path_length = strlen(path);
        char* plugin_entry_path = (char*)calloc(path_length + NV_PLUGIN_ENTRYPOINT_LENGTH + 1, sizeof(char)); // null terminator

        if (!plugin_entry_path) {
            return NV_ERR_MEM;
        }

        memcpy(plugin_entry_path, path, path_length); // base path prefix
        memcpy(plugin_entry_path + path_length, NV_PLUGIN_ENTRYPOINT, NV_PLUGIN_ENTRYPOINT_LENGTH); // entrypoint suffix

        if (stat(plugin_entry_path, &sb) == -1) {
            free(plugin_entry_path);
            return NV_ERR;
        }

        if ((sb.st_mode & S_IFMT) == S_IFREG) {
            if (luaL_dofile(L, plugin_entry_path) == LUA_OK) {
                // TODO: LOG
                nv_log("lua entrypoint loaded succesfully\n");
            } else {
                const char* strerr = lua_tostring(L, -1);
                nv_log(strerr);
            }
        }

        free(plugin_entry_path);
        break;
    }

    default:
        return NV_ERR;
    }

    return NV_OK;
}

int luaopen_mylib(lua_State* L)
{
    luaL_openlibs(L);
    return 1;
}

int nvlua_main()
{
    lua_State* L = luaL_newstate(); /* opens Lua */
    luaopen_base(L); /* opens the basic library */
    luaopen_table(L); /* opens the table library */
    luaopen_io(L); /* opens the I/O library */
    luaopen_string(L); /* opens the string lib. */
    luaopen_math(L); /* opens the math lib. */

    lua_register(L, "echo", nv_log_lua);

    nv_load_plugin(L, "./plugload/");

    lua_close(L);
    return NV_OK;
}
#endif
