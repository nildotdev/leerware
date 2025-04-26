#pragma once
#include "../../utilities/memory.h"
#include "../datatypes/vector.h"
#include "../datatypes/color.h"

class CDebugOverlayGameSystem
{
public:
	void RenderWithoutDots(bool bShouldDot)
	{
		return MEM::CallVFunc<void, 10U>(this, bShouldDot);
	}

	void AddLineOverlay(const Vector_t& vecStart, const Vector_t& vecEnd, const Color_t& colColor, bool bNoDethTest, float flDuration)
	{
		return MEM::CallVFunc<void, 12U>(this, vecStart, vecEnd, colColor, bNoDethTest, static_cast<double>(flDuration));
	}

	void AddBoxOverlay(const Vector_t& vecOrigin, const Vector_t& vecMins, const Vector_t& vecMaxs, const QAngle_t& angRotation, const Color_t& colColor, float flDuration)
	{
		return MEM::CallVFunc<void, 48U>(this, vecOrigin, vecMins, vecMaxs, angRotation, colColor[COLOR_R], colColor[COLOR_G], colColor[COLOR_B], colColor[COLOR_A], static_cast<double>(flDuration));
	}

	void AddTextOverlay(const Vector_t& vecOrigin, float flDuration, int nLineOffset, const char* szText)
	{
		return MEM::CallVFunc<void, 58U>(this, vecOrigin, nLineOffset, static_cast<double>(flDuration), szText);
	}

	void AddTextOverlay(const Vector_t& vecOrigin, float flDuration, const char* szText)
	{
		return MEM::CallVFunc<void, 59U>(this, vecOrigin, static_cast<double>(flDuration), szText);
	}

	void AddTextOverlay(const Vector_t& vecOrigin, float flDuration, int nLineOffset, const Color_t& colColor, const char* szText)
	{
		return MEM::CallVFunc<void, 60U>(this, vecOrigin, nLineOffset, static_cast<double>(flDuration), colColor[COLOR_R], colColor[COLOR_G], colColor[COLOR_B], colColor[COLOR_A], szText);
	}
};
