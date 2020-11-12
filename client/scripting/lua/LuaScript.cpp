#include "main.h"

extern "C"
{
#include "Lua/include/lua.h"
#include "Lua/include/lualib.h"
#include "Lua/include/lauxlib.h"
}

#ifdef _WIN32
#pragma comment(lib, "Lua/lua5.1.lib")
#endif

int luaPrint(lua_State *L)
{
	const char *readString = luaL_checkstring(L, 1);
	printf("%s\n", readString);
	return 0;
}

void LuaScript::Init()
{
	printf("Lua scripting language loaded\n");
	
	/*std::string cmd = "a = 7 + 11";

	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	lua_pushcfunction(L, luaPrint);
	lua_setglobal(L, "printf");
	char * code = "printf('string z lua')";
	luaL_dostring(L, code);
	lua_close(L);*/
}