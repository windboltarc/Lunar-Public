#pragma once
#include <cstdint>
#include <string>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <Xorstr/xorstr.hpp>
#include "enchelper/enchelper.hpp"

struct lua_State;

#define rebase(x) (x + reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr)))
#define rebase_hyperion(x) (x + reinterpret_cast<uintptr_t>(GetModuleHandleA(xorstr_("RobloxPlayerBeta.dll"))))

namespace update
{
    const uintptr_t bitmap = rebase_hyperion(0x26DAB8);

    namespace roblox
    {
        const uintptr_t PropertyDescriptorBitFlags = 0x50;
        const uintptr_t ScriptableMask = 0x20;
        const uintptr_t print = rebase(0x14CA3E0);
        const uintptr_t get_lua_state = rebase(0xB4F910);
        const uintptr_t luad_throw = rebase(0x27766C0);
        const uintptr_t impersonator = rebase(0x38F31B0);
        const uintptr_t push_instance = rebase(0xE9CBA0);
        const uintptr_t get_property = rebase(0xAA72B0);
        const uintptr_t task_defer = rebase(0xFFBF40);
        const uintptr_t get_identity_struct = rebase(0x3ACE6D0);
        const uintptr_t fire_mouse_click = rebase(0x1BFD9B0);
        const uintptr_t fire_right_mouse_click = rebase(0x1BFDB50);
        const uintptr_t fire_mouse_hover_enter = rebase(0x1BFEF50);
        const uintptr_t fire_mouse_hover_leave = rebase(0x1BFF0F0);
        const uintptr_t fire_proximity_prompt = rebase(0x1CE90B0);
        const uintptr_t fire_touch_interest = rebase(0x1E43C80);
    }

    namespace lua
    {
        const uintptr_t luao_nilobject = rebase(0x4E52D48);
        const uintptr_t luau_execute = rebase(0x27A6DA0);
        const uintptr_t luah_dummynode = rebase(0x4E52768);
        const uintptr_t opcode_lookup = rebase(0x548C4C0);
    }

    namespace offsets
    {
        namespace datamodel
        {
            const uintptr_t fake_datamodel = rebase(0x70A86B8);
            const uintptr_t fake_datamodel_to_datamodel = 0x1C0;
            const uintptr_t script_context = 0x3D0;
            const uintptr_t game_loaded = 0x688;
            const uintptr_t identity_ptr = rebase(0x689b3c8);
            const uintptr_t raw_scheduler = rebase(0x7171238);
            const uintptr_t k_table = rebase(0x68667B0);
            const uintptr_t taskscheduler_target_fps = rebase(0x6866724);
            constexpr uintptr_t Overlap = 0x268;
        }

        namespace script
        {
            const uintptr_t localscript = 0x1B0;
            const uintptr_t modulescript = 0x158;
            const uintptr_t weak_thread_node = 0x188;
            const uintptr_t weak_thread_ref = 0x8;
            const uintptr_t weak_thread_ref_live = 0x20;
            const uintptr_t weak_thread_ref_live_thread = 0x8;
        }

        namespace instance
        {
            const uintptr_t name = 0x88;
            const uintptr_t children = 0x68;
            const uintptr_t class_name = 0x8;
            const uintptr_t class_descriptor = 0x18;
            const uintptr_t property_descriptor = 0x3C0;
            const uintptr_t primitive_touch = 0x178;
            const uintptr_t place_id = 0x1A0;
            const uintptr_t name_descriptor = 0x18;
            const uintptr_t callback_value = 0x88;
        }

        namespace extra_space
        {
            const uintptr_t identity = 0x30;
            const uintptr_t capabilities = 0x48;
        }
    }
}

namespace roblox
{
    using print_func_t = uintptr_t(__fastcall*)(int, const char*, ...);
    inline print_func_t r_print = reinterpret_cast<print_func_t>(update::roblox::print);

    using decrypt_state_t = uintptr_t(__fastcall*)(int64_t, uint64_t*, uint64_t*);
    inline decrypt_state_t get_state = reinterpret_cast<decrypt_state_t>(update::roblox::get_lua_state);

    using luad_throw_t = void(__fastcall*)(lua_State*, int);
    inline luad_throw_t luad_throw = reinterpret_cast<luad_throw_t>(update::roblox::luad_throw);

    inline auto getproperty = (uintptr_t * (__thiscall*)(uintptr_t, uintptr_t*))update::roblox::get_property;
    inline auto task_defer = (int(__fastcall*)(lua_State*))update::roblox::task_defer;
    inline auto impersonator = (void(__fastcall*)(lua_State*, int, uintptr_t))update::roblox::impersonator;
    inline auto push_instance = (void(__fastcall*)(lua_State*, uintptr_t))update::roblox::push_instance;
    inline auto luau_execute = (void(__fastcall*)(lua_State*))update::lua::luau_execute;
    inline auto get_identity_struct = (uintptr_t(__fastcall*)(uintptr_t))update::roblox::get_identity_struct;

    inline auto fire_mouse_click = (void(__fastcall*)(__int64 clickDetector, float distance, __int64 localPlayer))update::roblox::fire_mouse_click;
    inline auto fire_touch_interest = (void(__fastcall*)(__int64 clickDetector, float distance, __int64 localPlayer))update::roblox::fire_touch_interest;
    inline auto fire_mouse_hover_enter = (void(__fastcall*)(__int64 clickDetector, __int64 localPlayer))update::roblox::fire_mouse_hover_enter;
    inline auto fire_mouse_hover_leave = (void(__fastcall*)(__int64 clickDetector, __int64 localPlayer))update::roblox::fire_mouse_hover_leave;
    inline auto fire_right_mouse_click = (void(__fastcall*)(__int64 clickDetector, float distance, __int64 localPlayer))update::roblox::fire_right_mouse_click;
}

namespace Memory {
    inline void set_thread_identity(uintptr_t L, uintptr_t identity) {
        *reinterpret_cast<uint32_t*>(L + update::offsets::extra_space::identity) = static_cast<uint32_t>(identity);
    }

    inline void set_thread_capabilities(uintptr_t L, uintptr_t capabilities) {
        *reinterpret_cast<uintptr_t*>(L + update::offsets::extra_space::capabilities) = capabilities;
    }
}
#define LUAU_COMMA_SEP ,
#define LUAU_SEMICOLON_SEP ;

#define LUAU_SHUFFLE3(s, a1, a2, a3) a2 s a3 s a1
#define LUAU_SHUFFLE4(s, a1, a2, a3, a4) a1 s a2 s a4 s a3
#define LUAU_SHUFFLE5(s, a1, a2, a3, a4, a5) a1 s a5 s a2 s a3 s a4
#define LUAU_SHUFFLE6(s, a1, a2, a3, a4, a5, a6) a4 s a1 s a6 s a5 s a3 s a2
#define LUAU_SHUFFLE7(s, a1, a2, a3, a4, a5, a6, a7) a7 s a3 s a6 s a4 s a2 s a1 s a5
#define LUAU_SHUFFLE8(s, a1, a2, a3, a4, a5, a6, a7, a8) a1 s a8 s a4 s a6 s a2 s a7 s a5 s a3
#define LUAU_SHUFFLE9(s, a1, a2, a3, a4, a5, a6, a7, a8, a9) a1 s a5 s a3 s a2 s a4 s a9 s a8 s a7 s a6

#define PROTO_MEMBER1_ENC VMValue0
#define PROTO_MEMBER2_ENC VMValue2
#define PROTO_DEBUGISN_ENC VMValue4
#define PROTO_DEBUGNAME_ENC VMValue3
#define PROTO_TYPEINFO_ENC VMValue1

#define LSTATE_STACKSIZE_ENC VMValue1
#define LSTATE_GLOBAL_ENC VMValue0

#define CLOSURE_FUNC_ENC VMValue0
#define CLOSURE_CONT_ENC VMValue4
#define CLOSURE_DEBUGNAME_ENC VMValue2

#define TABLE_MEMBER_ENC VMValue0
#define TABLE_META_ENC VMValue0

#define UDATA_META_ENC VMValue4

#define TSTRING_HASH_ENC VMValue3
#define TSTRING_LEN_ENC VMValue0

#define GSTATE_TTNAME_ENC VMValue0
#define GSTATE_TMNAME_ENC VMValue0