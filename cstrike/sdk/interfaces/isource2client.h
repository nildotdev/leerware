#pragma once
#include "../../utilities/memory.h"

class CDebugOverlayGameSystem;
class ISource2Client
{
public:
	CDebugOverlayGameSystem* GetDebugOverlay()
	{
		return MEM::CallVFunc<CDebugOverlayGameSystem*, 163U>(this);
	}
};
