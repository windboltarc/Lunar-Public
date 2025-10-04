#include <lz4/include/lz4.h>
#include "lz4lib.hpp"

namespace lz4
{
    int lz4compress(lua_State* L) {
        luaL_trimstack(L, 1);
        luaL_checktype(L, 1, LUA_TSTRING);

        size_t len{};
        const char* source = lua_tolstring(L, 1, &len);

        const int max_compressed_sz = LZ4_compressBound(len);

        const auto buffer = new char[max_compressed_sz];
        memset(buffer, 0, max_compressed_sz);

        const auto actual_sz = LZ4_compress_default(source, buffer, len, max_compressed_sz);

        lua_pushlstring(L, buffer, actual_sz);
        return 1;
    }

    int lz4decompress(lua_State* L) {
        luaL_trimstack(L, 2);
        luaL_checktype(L, 1, LUA_TSTRING);
        luaL_checktype(L, 2, LUA_TNUMBER);

        size_t len{};
        const char* source = lua_tolstring(L, 1, &len);
        int data_sz = lua_tointeger(L, 2);

        char* buffer = new char[data_sz];

        memset(buffer, 0, data_sz);

        LZ4_decompress_safe(source, buffer, len, data_sz);

        lua_pushlstring(L, buffer, data_sz);
        return 1;
    }
}



void lz4_lib::initialize(lua_State* L)
{
    register_env_functions(L,
        {
            {"lz4compress",   lz4::lz4compress},
            {"lz4decompress", lz4::lz4decompress},

            {nullptr, nullptr}
        });
}
