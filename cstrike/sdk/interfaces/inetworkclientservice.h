#pragma once
#include "../../utilities/memory.h"

class CNetChannelInfo;
class CNetworkGameClient
{
public:
	MEM_PAD(0xE8); //0x0000
	CNetChannelInfo* pNetChannelInfo; // 0x00E8
	MEM_PAD(0x8); // 0x00F0
	bool bShouldPredict; // 0x00F8
	MEM_PAD(0x7B); // 0x00F9
	int nSomePredictionTick; // 0x0174
	MEM_PAD(0x104); // 0x0178
	int nDeltaTick; // 0x027C

	bool IsConnected()
	{
		return MEM::CallVFunc<bool, 12U>(this);
	}

	// force game to clear cache and reset delta tick
	void FullUpdate()
	{
		// @ida: #STR: "Requesting full game update (%s)...\n"
		MEM::CallVFunc<void, 28U>(this, CS_XOR("unk"));
	}

	int GetDeltaTick()
	{
		// @ida: offset in FullUpdate();
		// (nDeltaTick = -1) == FullUpdate() called
		return *reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + 0x25C);
	}

	float GetClientInterpAmount()
	{
		using fOriginal = float(CS_FASTCALL*)();
		static auto oOriginal = reinterpret_cast<fOriginal>(MEM::GetAbsoluteAddress(MEM::FindPattern(ENGINE2_DLL, CS_XOR("E8 ? ? ? ? 0F 2F F0 73 ?")), 0x1));
		return oOriginal();
	}
};

class INetworkClientService
{
public:
	[[nodiscard]] CNetworkGameClient* GetNetworkGameClient()
	{
		return MEM::CallVFunc<CNetworkGameClient*, 23U>(this);
	}
};
