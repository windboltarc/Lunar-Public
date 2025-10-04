#include "script_library.hpp"


int identifyexecutor(lua_State* L)
{
	lua_pushstring(L, "LunarBase");
	lua_pushstring(L, "1.0.0");
	return 2;
}

int getgenv(lua_State* L)
{
	lua_pushvalue(L, LUA_ENVIRONINDEX);
	return 1;
}

int getscriptbytecode(lua_State* L)
{
	if (lua_type(L, 1) != LUA_TUSERDATA) { lua_pushnil(L); return 1; }

	uintptr_t script = *(uintptr_t*)lua_touserdata(L, 1);
	if (!script) 
	{ 
		lua_pushnil(L); 
		return 1; 
	}

	lua_getfield(L, 1, "ClassName");
	const char* name = lua_tostring(L, -1);
	lua_pop(L, 1);

	uintptr_t addr = name && strcmp(name, "ModuleScript") == 0
		? *(uintptr_t*)(script + update::offsets::script::modulescript)
		: *(uintptr_t*)(script + update::offsets::script::localscript);

	std::string bytecode = global_functions::read_bytecode(addr);
	std::string code = addr ? global_functions::decompress_bytecode(bytecode) : "";
	if (code.empty()) 
	{ 
		lua_pushnil(L); 
		return 1; 
	}

	lua_pushlstring(L, code.data(), code.size());
	return 1;
}



int getscripts(lua_State* L) {
    struct instancecontext {
        lua_State* L;
        __int64 n;
    };

    instancecontext Context = { L, 0 };

    lua_createtable(L, 0, 0);

    const auto originalGCThreshold = L->global->GCthreshold;
    L->global->GCthreshold = SIZE_MAX;

    luaM_visitgco(L, &Context, [](void* ctx, lua_Page* page, GCObject* gco) -> bool {
        auto context = static_cast<instancecontext*>(ctx);
        lua_State* L = context->L;

        if (isdead(L->global, gco))
            return false;

        if (gco->gch.tt == LUA_TUSERDATA) {
            TValue* top = L->top;
            top->value.p = reinterpret_cast<void*>(gco);
            top->tt = LUA_TUSERDATA;
            L->top++;

            if (strcmp(luaL_typename(L, -1), "Instance") == 0) {
                lua_getfield(L, -1, "ClassName");
                const char* className = lua_tolstring(L, -1, 0);

                if (className && (
                    strcmp(className, "LocalScript") == 0 ||
                    strcmp(className, "ModuleScript") == 0 ||
                    strcmp(className, "Script") == 0))
                {
                    lua_pop(L, 1);

                    lua_getfield(L, -1, "Parent");
                    if (lua_isnil(L, -1)) {
                        lua_pop(L, 1);

                        context->n++;
                        lua_rawseti(L, -2, context->n);
                    }
                    else {
                        lua_pop(L, 2);
                    }
                }
                else {
                    lua_pop(L, 2);
                }
            }
            else {
                lua_pop(L, 1);
            }
        }

        return true;
        });

    L->global->GCthreshold = originalGCThreshold;

    return 1;
}
void script_library::initialize(lua_State* L)
{
	register_env_functions(L,
		{
            {"identifyexecutor", identifyexecutor},
            {"getgenv", getgenv},

            {"getscriptbytecode", getscriptbytecode},
            {nullptr, nullptr}
		});
}