// https://github.com/yasuoka/luacstruct
// https://www.lua.org/pil/contents.html

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nvlua.h"

static const struct luaL_Reg libnv[] = {
    { "dir", NULL },
    { NULL, NULL } /* sentinel */
};

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
        char* dotnv_path = (char*)malloc(strlen(path) + sizeof("dotnv"));
        nv_read_dotnv(dotnv_path);
        break;

    default:
        return;
    }
}

int luaopen_mylib(lua_State* L)
{
    luaL_openlib(L, "libnv", libnv, 0);
    return 1;
}

int main2(void)
{
    char buff[256];
    int error = 0;
    lua_State* L = lua_open(); /* opens Lua */
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
