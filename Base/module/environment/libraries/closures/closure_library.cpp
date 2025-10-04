#include "closure_library.hpp"

// the newcclosure and shit does not support yielding add it yourself!

static std::unordered_map<Closure*, Closure*> executor_closures;
static std::unordered_map<Closure*, Closure*> original_functions;

int loadstring(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TSTRING);

    const char* source = lua_tostring(L, 1);
    const char* chunk_name = luaL_optstring(L, 2, "LunarBase");

    const std::string& bytecode = global_functions::compile_script(source);

    if (luau_load(L, chunk_name, bytecode.data(), bytecode.size(), 0) != LUA_OK)
    {
        lua_pushnil(L);
        lua_pushvalue(L, -2);
        return 2;
    }

    if (Closure* func = lua_toclosure(L, -1))
    {
        if (func->l.p)
            context_manager::set_proto_capabilities(func->l.p, &max_caps);
    }

    lua_setsafeenv(L, LUA_GLOBALSINDEX, false);
    return 1;
}

static int lclosure_wrapper(lua_State* L) // hell naw
{
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L, 1);
    lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
    return lua_gettop(L);
}

int hookfunction(lua_State* L)
{
    lua_normalisestack(L, 2);
    luaL_checktype(L, 1, LUA_TFUNCTION);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    Closure* original = lua_toclosure(L, 1);
    Closure* replacement = lua_toclosure(L, 2);

    if (!original || !replacement)
        luaL_error(L, "wtf?");

    if (original->isC)
        lua_clonecfunction(L, 1);
    else
        lua_clonefunction(L, 1);

    Closure* original_cloned = clvalue(index2addr(L, -1));
    original_functions[original] = original_cloned;
    lua_pop(L, 1);

    if (original->isC && replacement->isC) // C->C
    {
        original->c.f = replacement->c.f;
        original->c.cont = replacement->c.cont;
        original->env = replacement->env;
        original->stacksize = replacement->stacksize;
        original->preload = replacement->preload;

        for (int i = 0; i < original->nupvalues; i++)
            setobj2n(L, &original->c.upvals[i], luaO_nilobject);

        for (int i = 0; i < replacement->nupvalues; i++)
            setobj2n(L, &original->c.upvals[i], &replacement->c.upvals[i]);
    }
    else if (!original->isC && !replacement->isC) // L->L
    {
        original->l.p = replacement->l.p;
        original->stacksize = replacement->stacksize;

        for (int i = 0; i < original->nupvalues; i++)
            setobj2n(L, &original->l.uprefs[i], luaO_nilobject);

        int count = min(original->nupvalues, replacement->nupvalues);
        for (int i = 0; i < count; i++)
            setobj(L, &original->l.uprefs[i], &replacement->l.uprefs[i]);
    }
    else if (original->isC && !replacement->isC) // C->L
    {
        lua_pushvalue(L, 2);
        lua_pushcclosure(L, lclosure_wrapper, nullptr, 1);
        Closure* wrapped = lua_toclosure(L, -1);

        original->c.f = wrapped->c.f;
        original->c.cont = wrapped->c.cont;
        original->env = wrapped->env;
        original->stacksize = wrapped->stacksize;
        original->preload = wrapped->preload;

        for (int i = 0; i < original->nupvalues; i++)
            setobj2n(L, &original->c.upvals[i], luaO_nilobject);

        for (int i = 0; i < wrapped->nupvalues; i++)
            setobj2n(L, &original->c.upvals[i], &wrapped->c.upvals[i]);

        lua_pop(L, 1);
    }
    else if (!original->isC && replacement->isC) // L->C
    {
        lua_newtable(L);
        lua_newtable(L);
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        lua_setfield(L, -2, "__index");
        lua_setmetatable(L, -2);
        lua_pushvalue(L, 2);
        lua_setfield(L, -2, "tramp");

        const char* src = "return tramp(...)";
        const std::string& bc = global_functions::compile_script(src);
        if (luau_load(L, "@LunarBase", bc.data(), bc.size(), 0) != LUA_OK)
        {
            lua_pop(L, 2);
            luaL_error(L, "Failed to compile huh??");
        }
        Closure* thunk = lua_toclosure(L, -1);

        thunk->env = hvalue(luaA_toobject(L, -2));
        luaC_threadbarrier(L);

        if (thunk->l.p)
            context_manager::set_proto_capabilities(thunk->l.p, &max_caps);

        original->l.p = thunk->l.p;
        original->env = thunk->env;

        for (int i = 0; i < original->nupvalues; i++)
            setobj2n(L, &original->l.uprefs[i], luaO_nilobject);

        for (int i = 0; i < replacement->nupvalues; i++)
            setobj2n(L, &original->l.uprefs[i], &replacement->c.upvals[i]);

        lua_pop(L, 2);
    }
    else
    {
        luaL_error(L, "hooking type not supported");
    }

    Closure* cloned = original_functions[original];
    setclvalue(L, L->top, cloned);
    L->top += 1;

    return 1;
}

int restorefunction(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);

    Closure* function = lua_toclosure(L, 1);
    if (!function)
        luaL_error(L, "invalid function");

    auto it = original_functions.find(function);
    if (it == original_functions.end())
        luaL_error(L, "closure is not hooked");

    Closure* original = it->second;

    if (function->isC) // cclosure
    {
        function->c.f = original->c.f;
        function->c.cont = original->c.cont;
        function->env = original->env;
        function->stacksize = original->stacksize;
        function->preload = original->preload;

        for (int i = 0; i < function->nupvalues; i++)
            setobj2n(L, &function->c.upvals[i], luaO_nilobject);

        for (int i = 0; i < original->nupvalues; i++)
            setobj2n(L, &function->c.upvals[i], &original->c.upvals[i]);
    }
    else if (!function->isC) // lclosure
    {
        function->l.p = original->l.p;
        function->stacksize = original->stacksize;

        for (int i = 0; i < function->nupvalues; i++)
            setobj2n(L, &function->l.uprefs[i], luaO_nilobject);

        int count = min(function->nupvalues, original->nupvalues);
        for (int i = 0; i < count; i++)
            setobj(L, &function->l.uprefs[i], &original->l.uprefs[i]);
    }

    original_functions.erase(function);
    return 0;
}

int clonefunction(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    Closure* f = clvalue(index2addr(L, 1));
    if (!f) luaL_error(L, "idk huh?");

    f->isC ? lua_clonecfunction(L, 1) : lua_clonefunction(L, 1);
    return 1;
}

int iscclosure(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushboolean(L, clvalue(index2addr(L, 1))->isC);
    return 1;
}

int islclosure(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushboolean(L, !clvalue(index2addr(L, 1))->isC);
    return 1;
}

int newcclosure(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);
    Closure* f = clvalue(index2addr(L, 1));

    if (!f) 
        luaL_error(L, "invalid closure");

    lua_pushvalue(L, 1);
    if (!f->isC) 
        lua_pushcclosure(L, lclosure_wrapper, nullptr, 1);

    return 1;
}

int isexecutorclosure(lua_State* L)
{
    if (lua_type(L, 1) != LUA_TFUNCTION) { lua_pushboolean(L, 0); return 1; }

    Closure* f = lua_toclosure(L, 1);
    bool ex = !f->isC ? (f->l.p && f->l.p->linedefined != 0) : get_closures().count(f) > 0;

    lua_pushboolean(L, ex);
    return 1;
}

int getrenv(lua_State* L) {
    luaL_trimstack(L, 0);
    lua_State* RobloxState = globals::lunar_state;
    LuaTable* clone = luaH_clone(L, RobloxState->gt);

    lua_rawcheckstack(L, 1);
    luaC_threadbarrier(L);
    luaC_threadbarrier(RobloxState);

    L->top->value.p = clone;
    L->top->tt = LUA_TTABLE;
    L->top++;

    lua_rawgeti(L, LUA_REGISTRYINDEX, 2);
    lua_setfield(L, -2, "_G");
    lua_rawgeti(L, LUA_REGISTRYINDEX, 4);
    lua_setfield(L, -2, "shared");
    return 1;
}
int checkcaller(lua_State* L) {
    const auto script_ptr = *(std::uintptr_t*)((std::uintptr_t)((L->userdata)) + 0x50);
    lua_pushboolean(L, !script_ptr);
    return 1;
}
namespace HelpFuncs {
    static void IsInstance(lua_State* L, int idx) {
        std::string typeoff = luaL_typename(L, idx);
        if (typeoff != "Instance")
            luaL_typeerrorL(L, 1, "Instance");
    };
}

int cloneref(lua_State* LS) {
    luaL_checktype(LS, 1, LUA_TUSERDATA);
    HelpFuncs::IsInstance(LS, 1);
    const auto OldUserdata = lua_touserdata(LS, 1);
    const auto NewUserdata = *reinterpret_cast<uintptr_t*>(OldUserdata);

    lua_pushlightuserdata(LS, (void*)roblox::push_instance);

    lua_rawget(LS, -10000);
    lua_pushlightuserdata(LS, reinterpret_cast<void*>(NewUserdata));
    lua_rawget(LS, -2);

    lua_pushlightuserdata(LS, reinterpret_cast<void*>(NewUserdata));
    lua_pushnil(LS);
    lua_rawset(LS, -4);

    roblox::push_instance(LS, (uintptr_t)OldUserdata);

    lua_pushlightuserdata(LS, reinterpret_cast<void*>(NewUserdata));
    lua_pushvalue(LS, -3);
    lua_rawset(LS, -5);

    return 1;
};

void closure_library::initialize(lua_State* L)
{
	register_env_functions(L,
		{
			{"loadstring", loadstring},
			{"cloneref", cloneref},

			{"getrenv", getrenv},

            {"hookfunction", hookfunction},
            {"replaceclosure", hookfunction},
            {"restorefunction", restorefunction},

            {"clonefunction", clonefunction},

            {"iscclosure", iscclosure},
            {"islclosure", islclosure},

            {"isexecutorclosure", isexecutorclosure},
            {"checkclosure", isexecutorclosure},
            {"isourclosure", isexecutorclosure},

            {"newcclosure", newcclosure},
            {"checkcaller", checkcaller},
			{nullptr, nullptr}
		});
}