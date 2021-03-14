#pragma once

#if (defined(WIN32) || defined(_WIN32)) && !defined(UCMAKE_STATIC_UDLua_core)
#ifdef UCMAKE_EXPORT_UDLua_core
#define UDLua_core_API __declspec(dllexport)
#else
#define UDLua_core_API __declspec(dllimport)
#endif
#else
#define UDLua_core_API extern
#endif // (defined(WIN32) || defined(_WIN32)) && !defined(UCMAKE_STATIC_UDLua_core)

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

// Name
// Type
// ObjectView
// SharedObject
// UDRefl
UDLua_core_API void luaopen_UDLua(lua_State* L);

#ifdef __cplusplus
}
#endif // __cplusplus
