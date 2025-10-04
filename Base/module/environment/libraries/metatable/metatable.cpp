#include "metatable.hpp"
#include "VM/include/lua.h"
#include "VM/include/lualib.h"
#include "VM/src/lcommon.h"
#include "VM/src/lstring.h"
#include "VM/src/lfunc.h"
#include "VM/src/lmem.h"
#include "VM/src/lgc.h"
#include "VM/src/ltable.h"
#include "VM/src/lobject.h"
#include "VM/src/lstate.h"
#include "VM/src/lapi.h"
#include "VM/src/ldo.h"

namespace Metatable {
    int getrawmetatable(lua_State* L) {
        luaL_trimstack(L, 1);
        luaL_checkany(L, 1);
        if (!lua_getmetatable(L, 1))
            lua_pushnil(L);
        return 1;
    }

    int setrawmetatable(lua_State* L) {
        luaL_trimstack(L, 2);
        luaL_checkany(L, 1);
        luaL_checktype(L, 2, LUA_TTABLE);
        lua_setmetatable(L, 1);
        lua_pushvalue(L, 1);
        return 1;
    }

    int setreadonly(lua_State* L) {
        luaL_trimstack(L, 2);
        luaL_checktype(L, 1, LUA_TTABLE);
        luaL_checktype(L, 2, LUA_TBOOLEAN);
        hvalue(luaA_toobject(L, 1))->readonly = lua_toboolean(L, 2);
        return 0;
    }

    int isreadonly(lua_State* L) {
        luaL_trimstack(L, 1);
        luaL_checktype(L, 1, LUA_TTABLE);
        lua_pushboolean(L, hvalue(luaA_toobject(L, 1))->readonly);
        return 1;
    }
}

void metatablelib::initialize(lua_State* L)
{
    register_env_functions(L,
        {
            {"getrawmetatable", Metatable::getrawmetatable},
            {"setrawmetatable", Metatable::setrawmetatable},
            {"setreadonly",     Metatable::setreadonly},
            {"isreadonly",      Metatable::isreadonly},

            {nullptr, nullptr}
        });
}
