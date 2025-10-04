#include "task_scheduler.hpp"
#include <sstream>
#include <queue>
#include <functional>
#include <optional>

int hook_scheduler(lua_State* L) {
    if (!globals::execution_queue.empty()) {
        auto script = std::move(globals::execution_queue.front());
        globals::execution_queue.pop();
        execution::execute_script(globals::lunar_state, std::move(script));
    }
    return 0;
}

void setup_queue(lua_State* L)
{
    lua_getglobal(L, "game");
    lua_getfield(L, -1, "GetService");
    lua_pushvalue(L, -2);

    lua_pushstring(L, "RunService");
    lua_pcall(L, 2, 1, 0);

    lua_getfield(L, -1, "Heartbeat");
    lua_getfield(L, -1, "Connect");
    lua_pushvalue(L, -2);

    lua_pushcclosure(L, hook_scheduler, nullptr, 0);
    lua_pcall(L, 2, 0, 0);
    lua_pop(L, 2);
}

void task_scheduler::initialize_scheduler() {
    globals::roblox_state = reinterpret_cast<lua_State*>(context_manager::get_lua_state());
    globals::lunar_state = lua_newthread(globals::roblox_state);

    context_manager::set_thread_capabilities(globals::lunar_state, 8, max_caps);
    environment::initialize(globals::lunar_state);

    setup_queue(globals::lunar_state);

    task_scheduler::send_script(R"--(
        printidentity()
        print("Lunar has loaded!")
    )--");
    task_scheduler::send_script("loadstring(game:HttpGet('https://raw.githubusercontent.com/fluxendo902/drawinglibrary/refs/heads/main/e', true))()");//Here IS DrawingLib Function
    
    task_scheduler::send_script(R"IMGUI(

local RunService = game:GetService("RunService")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local IsStudio = RunService:IsStudio()

local ImGui
if IsStudio then
	ImGui = require(ReplicatedStorage.ImGui)
else
	local SourceURL = 'https://raw.githubusercontent.com/depthso/Roblox-ImGUI/refs/heads/main/ImGui.lua'
	ImGui = loadstring(game:HttpGet(SourceURL))()
end

local Window = ImGui:CreateWindow({
	Title = "Lunar Base",
	Size = UDim2.new(0, 350, 0, 370),
	Position = UDim2.new(0.5, 0, 0, 70)
})
Window:Center()

local main = Window:CreateTab({
	Name = "main",
})

local Key = main:InputText({
	Label = "",
	PlaceHolder = "--main functions by LunarCommunity",
	Value = "",
	Size = UDim2.new(1, 0, 0.9, 0),
})

local row = main:Row()

row:Button({
	Text = "Execute",
	Callback = function()
		if Key:GetValue() then
			local script = Key:GetValue()
            local result = pcall(loadstring(script))
		else
			print('Error executing script: '.. tostring(result))
		end
	end,
})

row:Button({
	Text = "Clear",
	Callback = function()
		Key:SetValue("")
	end,
})
)IMGUI");

}

void task_scheduler::send_script(const std::string& script) {
    if (!script.empty()) {
        globals::execution_queue.push(script);
    }
}