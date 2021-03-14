#include <UDLua/UDLua.h>

#include <iostream>

int main() {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaopen_UDLua(L);

	char buffer[256];
	while (std::fgets(buffer, sizeof(buffer), stdin) != nullptr) {
		int error = luaL_loadstring(L, buffer) || lua_pcall(L, 0, 0, 0);
		if (error) {
			std::cerr << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}
	}

	lua_close(L);
	return 0;
}
