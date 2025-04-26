#pragma once

// used: MEM::CallVFunc
#include "../../utilities/memory.h"

class CPVS
{
public:
	void Set(bool bState)
	{
		MEM::CallVFunc<void*, 6U>(this, bState);
	}
};
