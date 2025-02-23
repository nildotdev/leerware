#pragma once

// used: mem_pad
#include "../../utilities/memory.h"

class IGlobalVars
{
public:
	float flRealTime; //0x0000
	int32_t nFrameCount; //0x0004
	float flFrameTime; //0x0008
	float flFrameTime2; //0x000C
	int32_t nMaxClients; //0x0010
	MEM_PAD(0x8); //0x0014
	float flIntervalPerTick; //0x001C
	MEM_PAD(0x10); //0x0020
	float flFrameTime3; //0x0030
	float flCurrentTime; //0x0034
	float flCurrentTime1; //0x0038
	float flTickFraction; //0x003C
	float flTickFraction1; //0x0040
	MEM_PAD(0x4); //0x0044
	int32_t nCurrentTick; //0x0048
	int32_t nTickCount; //0x004C
	float flLastFrameTime; //0x0050
	MEM_PAD(0x4); //0x0054
	void* pCurrentNetChannel; //0x0058
	MEM_PAD(0x118); //0x0060
	char* szCurrentMapPath; //0x0178
	char* szCurrentMapName; //0x0180
};
