#include <iostream>
#include <Windows.h>
#include <thread>
#include <chrono>

#include "misc/teleport_handler/tp_handler.hpp"
#include "misc/communication/com.hpp"

#pragma region DLL_EXPORTS
extern "C" __declspec(dllexport) LRESULT NextHook(int code, WPARAM wParam, LPARAM lParam) { return CallNextHookEx(nullptr, code, wParam, lParam); }
#pragma endregion

void load() {
    teleport_handler::initialize();

    script_server server;
    if (!server.initialize(2304)) {
        roblox::r_print(0, "Failed-Sr-Baby");
        return;
    }

    while (true) {
        std::string script = server.receive_script();
        if (!script.empty()) {
            task_scheduler::send_script(script);
        }
        else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        // only uncomment this if the module is crashing after 30 secs (means your injector is pretty ass)
        // global_functions::patch_control_flow_guard(mod);

        std::thread(load).detach();
    }
    return TRUE;
}

// LunarCommunity.... http://dsc.gg/lunarexploit
