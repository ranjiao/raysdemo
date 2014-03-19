#include <stdio.h>
#include <string.h>
#include "state.h"

extern "C"
{
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>

  extern int luaopen_example(lua_State* L);
}

int dostring(lua_State *L, char* str) {
  int ok;
  ok=luaL_dostring(L,str);
  if (ok!=0)
    printf("[C] ERROR in dostring: %s\n",lua_tostring(L,-1));
  return ok;
}

int main()
{
  lua_State * L = lua_open(); /* opens Lua */
  luaL_openlibs(L);
  luaopen_example(L);

  if (luaL_loadfile(L, "test.lua") || lua_pcall(L, 0, 0, 0)) {
    printf("[C] ERROR: cannot run lua file: %s",lua_tostring(L, -1));
    return 3;
  }
  lua_close(L);
  return 0;
}