// https://github.com/yasuoka/luacstruct
// https://www.lua.org/pil/contents.html

#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lualib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nvlua.h"

static void nv_read_dotnv(char* path);
static void nv_open_plugin(char* path);
//  static void nv_find_plugins();
//  static void nv_make_plugin_directory();

static void nv_read_dotnv(char* path)
{
}

static void nv_open_plugin(char* path)
{
    if (!path) {
        return;
    }

    struct stat sb;
    if (stat(path, &sb) == -1) {
        return;
    }

    switch (sb.st_mode & S_IFMT) {
    case S_IFDIR:
        // find 'dotnv'
        // read entry point
        size_t dotnv_path_sz = strlen(path) + sizeof("dotnv");
        char* dotnv_path = (char*)malloc(dotnv_path_sz);
        snprintf(dotnv_path, dotnv_path_sz, "%s/%s", path, "plugin.lua");
        nv_read_dotnv(dotnv_path);
        free(dotnv_path);
        break;

    default:
        return;
    }
}

int luaopen_mylib(lua_State* L)
{
    luaL_openlibs(L);
    return 1;
}

int main2(void)
{
    char buff[256];
    int error = 0;
    lua_State* L = luaL_newstate(); /* opens Lua */
    luaopen_base(L); /* opens the basic library */
    luaopen_table(L); /* opens the table library */
    luaopen_io(L); /* opens the I/O library */
    luaopen_string(L); /* opens the string lib. */
    luaopen_math(L); /* opens the math lib. */

    while (fgets(buff, sizeof(buff), stdin) != NULL) {
        error = luaL_loadbuffer(L, buff, strlen(buff), "line") || lua_pcall(L, 0, 0, 0);
        if (error) {
            fprintf(stderr, "%s", lua_tostring(L, -1));
            lua_pop(L, 1); /* pop error message from the stack */
        }
    }

    nv_open_plugin(NULL);

    lua_close(L);
    return 0;
}
