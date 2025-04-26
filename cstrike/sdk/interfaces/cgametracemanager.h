#pragma once
// used: pad and findpattern
#include "../../utilities/memory.h"
// used: vector
#include "../../sdk/datatypes/vector.h"
// used: array
#include <array>

struct Ray_t
{
public:
	Vector_t m_vecStart;
	Vector_t m_vecEnd;
	Vector_t m_vecMins;
	Vector_t m_vecMaxs;
	MEM_PAD(0x4);
	std::uint8_t UnkType;
};
static_assert(sizeof(Ray_t) == 0x38);

struct SurfaceData_t
{
public:
	MEM_PAD(0x8)
	float m_flPenetrationModifier;
	float m_flDamageModifier;
	MEM_PAD(0x4)
	int m_iMaterial;
};

static_assert(sizeof(SurfaceData_t) == 0x18);

struct TraceHitboxData_t
{
public:
	char* m_pName; //0x0000
	char* m_pSurfaceProperties; //0x0008
	char* m_pBoneName; //0x0010
	char pad_0018[32]; //0x0018
	int32_t m_nHitGroup; //0x0038
	char pad_003C[12]; //0x003C
	int16_t m_nHitboxId; //0x0048
	char pad_004A[38]; //0x004A
};
static_assert(sizeof(TraceHitboxData_t) == 0x70);

class C_CSPlayerPawn;
struct GameTrace_t
{
public:
	GameTrace_t() = default;

	SurfaceData_t* GetSurfaceData();
	int GetHitboxId();
	int GetHitgroup();
	bool IsVisible() const;

	void* m_pSurface;
	C_CSPlayerPawn* m_pHitEntity;
	TraceHitboxData_t* m_pHitboxData;
	MEM_PAD(0x38);
	std::uint32_t m_uContents;
	MEM_PAD(0x24);
	Vector_t m_vecStartPos;
	Vector_t m_vecEndPos;
	Vector_t m_vecNormal;
	Vector_t m_vecPosition;
	MEM_PAD(0x4);
	float m_flFraction;
	MEM_PAD(0x6);
	bool m_bAllSolid;
	MEM_PAD(0x4D)
}; // Size: 0x108

static_assert(sizeof(GameTrace_t) == 0x108);

struct TraceFilter_t
{
public:
	MEM_PAD(0x8);
	std::int64_t m_uTraceMask;
	std::array<std::int64_t, 2> m_v1;
	std::array<std::int32_t, 4> m_arrSkipHandles;
	std::array<std::int16_t, 2> m_arrCollisions;
	std::int16_t m_v2;
	std::uint8_t m_v3;
	std::uint8_t m_v4;
	std::uint8_t m_v5;

	TraceFilter_t() = default;
	TraceFilter_t(std::uint64_t uMask, C_CSPlayerPawn* pSkip1, C_CSPlayerPawn* pSkip2, int nLayer);
};
static_assert(sizeof(TraceFilter_t) == 0x40);


struct UpdateValue_t
{
	float previousLengthMod;
	float currentLengthMod;
	MEM_PAD(0x8);
	int16_t handleIndex;
	MEM_PAD(0x6);
};

struct TraceArrElement_t
{
	MEM_PAD(0x30);
};

struct TraceData_t
{
	std::int32_t nUnk1;
	float flUnk2 = 52.0f;
	void* pArrPointer;
	std::int32_t nUnk3 = 128U;
	std::int32_t nUnk4 = static_cast<std::int32_t>(0x80000000);
	std::array<TraceArrElement_t, 0x80> mArr = {};
	MEM_PAD(0x8);
	std::int64_t nNumUpdate;
	void* pPointerUpdateValue;
	MEM_PAD(0xC8);
	Vector_t vecStart;
	Vector_t vecEnd;
	MEM_PAD(0x50);
};

class CGameTraceManager
{
public:
	bool TraceShape(Ray_t* pRay, Vector_t vecStart, Vector_t vecEnd, TraceFilter_t* pFilter, GameTrace_t* pGameTrace)
	{
		using fnTraceShape = bool(__fastcall*)(CGameTraceManager*, Ray_t*, Vector_t*, Vector_t*, TraceFilter_t*, GameTrace_t*);
		// Credit: https://www.unknowncheats.me/forum/4265752-post6333.html
		static fnTraceShape oTraceShape = reinterpret_cast<fnTraceShape>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 20 48 89 4C 24 08 55 56 41")));

		#ifdef CS_PARANOID
		CS_ASSERT(oTraceShape != nullptr);
		#endif

		return oTraceShape(this, pRay, &vecStart, &vecEnd, pFilter, pGameTrace);
	}

	bool ClipRayToEntity(Ray_t* pRay, Vector_t vecStart, Vector_t vecEnd, C_CSPlayerPawn* pPawn, TraceFilter_t* pFilter, GameTrace_t* pGameTrace)
	{
		using fnClipRayToEntity = bool(__fastcall*)(CGameTraceManager*, Ray_t*, Vector_t*, Vector_t*, C_CSPlayerPawn*, TraceFilter_t*, GameTrace_t*);
		static fnClipRayToEntity oClipRayToEntity = reinterpret_cast<fnClipRayToEntity>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 54 41 56 41 57 48 81 EC C0 00 00 00 48 8B 9C")));

		#ifdef CS_PARANOID
		CS_ASSERT(oClipRayToEntity != nullptr);
		#endif

		return oClipRayToEntity(this, pRay, &vecStart, &vecEnd, pPawn, pFilter, pGameTrace);
	}

	bool HandleBulletPenetration(TraceData_t* trace, void* stats, UpdateValue_t* mod_value, bool draw_showimpacts = false)
	{
		using fnHandleBulletPenetration = bool(CS_FASTCALL*)(TraceData_t*, void*, UpdateValue_t*, void*, void*, void*, void*, void*, bool);
		static fnHandleBulletPenetration oHandleBulletPenetration = reinterpret_cast<fnHandleBulletPenetration>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B C4 44 89 48 20 48 89 50 10 48")));

		#ifdef CS_PARANOID
		CS_ASSERT(oHandleBulletPenetration != nullptr);
		#endif

		return oHandleBulletPenetration(trace, stats, mod_value, nullptr, nullptr, nullptr, nullptr, nullptr, draw_showimpacts);
	}

	void Init(TraceFilter_t& filter, C_CSPlayerPawn* skip, uint64_t mask, uint8_t layer, uint16_t unk)
	{
		using fnTraceInit = TraceFilter_t*(CS_FASTCALL*)(TraceFilter_t&, C_CSPlayerPawn*, uint64_t, uint8_t, uint16_t);
		static fnTraceInit oTraceInit = reinterpret_cast<fnTraceInit>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 0F B6 41 37 33")));

		#ifdef CS_PARANOID
		CS_ASSERT(oTraceInit != nullptr);
		#endif

		oTraceInit(filter, skip, mask, layer, unk);
	}

	void CreateTrace(TraceData_t* trace, Vector_t start, Vector_t end, TraceFilter_t& filter, int penetrationCount)
	{
		using fnCreateTrace = bool(CS_FASTCALL*)(TraceData_t*, Vector_t, Vector_t, TraceFilter_t, void*, void*, void*, void*, int);
		static fnCreateTrace oCreateTrace = reinterpret_cast<fnCreateTrace>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 40 F2")));

		#ifdef CS_PARANOID
		CS_ASSERT(oCreateTrace != nullptr);
		#endif

		oCreateTrace(trace, start, end, filter, nullptr, nullptr, nullptr, nullptr, penetrationCount);
	}

	void InitTraceInfo(GameTrace_t* hit)
	{
		using fnInitTraceInfo = bool(CS_FASTCALL*)(GameTrace_t*);
		static fnInitTraceInfo oInitTraceInfo = reinterpret_cast<fnInitTraceInfo>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 33 FF 48 8B 0D ? ? ? ? 48 85 C9")));

		#ifdef CS_PARANOID
		CS_ASSERT(oInitTraceInfo != nullptr);
		#endif

		oInitTraceInfo(hit);
	}

	void GetTraceInfo(TraceData_t* trace, GameTrace_t* hit, float unk, void* unk1)
	{
		using fnGetTraceInfo = bool(CS_FASTCALL*)(TraceData_t*, GameTrace_t*, float, void*);
		static fnGetTraceInfo oGetTraceInfo = reinterpret_cast<fnGetTraceInfo>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 60 48 8B E9 0F")));

		#ifdef CS_PARANOID
		CS_ASSERT(oGetTraceInfo != nullptr);
		#endif

		oGetTraceInfo(trace, hit, unk, unk1);
	}
};
