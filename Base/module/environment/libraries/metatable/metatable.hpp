#pragma once
#include "../../environment.hpp"
#include "../../execution/execution.hpp"

#include <VM/src/lstate.h>

class metatablelib
{
public:
	static void initialize(lua_State* L);
};