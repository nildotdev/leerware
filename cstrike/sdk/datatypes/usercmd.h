#pragma once

// used: QAngle_t
#include "qangle.h"
// used: MEM_PAD
#include "../../utilities/memory.h"
// used: memalloc
#include "../../core/interfaces.h"
#include "../interfaces/imemalloc.h"

// @source: server.dll
enum ECommandButtons : std::uint64_t
{
	IN_ATTACK = 1 << 0,
	IN_JUMP = 1 << 1,
	IN_DUCK = 1 << 2,
	IN_FORWARD = 1 << 3,
	IN_BACK = 1 << 4,
	IN_USE = 1 << 5,
	IN_LEFT = 1 << 7,
	IN_RIGHT = 1 << 8,
	IN_MOVELEFT = 1 << 9,
	IN_MOVERIGHT = 1 << 10,
	IN_SECOND_ATTACK = 1 << 11,
	IN_RELOAD = 1 << 13,
	IN_SPEED = 1 << 16,
	IN_JOYAUTOSPRINT = 1 << 17,
	IN_SHOWSCORES = 1ULL << 33,
	IN_ZOOM = 1ULL << 34,
	IN_LOOKATWEAPON = 1ULL << 35
};

// compiled protobuf messages and looked at what bits are used in them
enum ESubtickMoveStepBits : std::uint32_t
{
	MOVESTEP_BITS_BUTTON = 0x1U,
	MOVESTEP_BITS_PRESSED = 0x2U,
	MOVESTEP_BITS_WHEN = 0x4U,
	MOVESTEP_BITS_ANALOG_FORWARD_DELTA = 0x8U,
	MOVESTEP_BITS_ANALOG_LEFT_DELTA = 0x10U
};

enum EInputHistoryBits : std::uint32_t
{
	INPUT_HISTORY_BITS_VIEWANGLES = 0x1U, // seems fine
	INPUT_HISTORY_BITS_SHOOTPOSITION = 0x20U, // FIXED . ,
	INPUT_HISTORY_BITS_TARGETHEADPOSITIONCHECK = 0x4U, // dont care
	INPUT_HISTORY_BITS_TARGETABSPOSITIONCHECK = 0x8U, //dont care
	INPUT_HISTORY_BITS_TARGETANGCHECK = 0x10U, // dont care
	INPUT_HISTORY_BITS_CL_INTERP = 0x10U, // Fixed.
	INPUT_HISTORY_BITS_SV_INTERP0 = 0x4U, // Fixed.
	INPUT_HISTORY_BITS_SV_INTERP1 = 0x8U, // fixed.
	INPUT_HISTORY_BITS_PLAYER_INTERP = 0x10U, // fixed.
	INPUT_HISTORY_BITS_RENDERTICKCOUNT = 0x200U, // correct.
	INPUT_HISTORY_BITS_RENDERTICKFRACTION = 0x400U, // dont care
	INPUT_HISTORY_BITS_PLAYERTICKCOUNT = 0x800U, // correct
	INPUT_HISTORY_BITS_PLAYERTICKFRACTION = 0x1000U, // dont care
	INPUT_HISTORY_BITS_FRAMENUMBER = 0x2000U, // dont care
	INPUT_HISTORY_BITS_TARGETENTINDEX = 0x4000U // correct.
};

enum EButtonStatePBBits : uint32_t
{
	BUTTON_STATE_PB_BITS_BUTTONSTATE1 = 0x1U,
	BUTTON_STATE_PB_BITS_BUTTONSTATE2 = 0x2U,
	BUTTON_STATE_PB_BITS_BUTTONSTATE3 = 0x4U
};

enum EBaseCmdBits : std::uint32_t
{
	BASE_BITS_MOVE_CRC = 0x1U,
	BASE_BITS_BUTTONPB = 0x2U,
	BASE_BITS_VIEWANGLES = 0x4U,
	BASE_BITS_COMMAND_NUMBER = 0x8U,
	BASE_BITS_CLIENT_TICK = 0x10U,
	BASE_BITS_FORWARDMOVE = 0x20U,
	BASE_BITS_LEFTMOVE = 0x40U,
	BASE_BITS_UPMOVE = 0x80U,
	BASE_BITS_IMPULSE = 0x100U,
	BASE_BITS_WEAPON_SELECT = 0x200U,
	BASE_BITS_RANDOM_SEED = 0x400U,
	BASE_BITS_MOUSEDX = 0x800U,
	BASE_BITS_MOUSEDY = 0x1000U,
	BASE_BITS_CONSUMED_SERVER_ANGLE = 0x2000U,
	BASE_BITS_CMD_FLAGS = 0x4000U,
	BASE_BITS_ENTITY_HANDLE = 0x8000U
};

enum ECSGOUserCmdBits : std::uint32_t
{
	CSGOUSERCMD_BITS_BASECMD = 0x1U,
	CSGOUSERCMD_BITS_LEFTHAND = 0x2U,
	CSGOUSERCMD_BITS_ATTACK3START = 0x4U,
	CSGOUSERCMD_BITS_ATTACK1START = 0x8U,
	CSGOUSERCMD_BITS_ATTACK2START = 0x10U
};

template <typename T>
struct RepeatedPtrField_t
{
	struct Rep_t
	{
		int nAllocatedSize;
		T* tElements[(std::numeric_limits<int>::max() - 2 * sizeof(int)) / sizeof(void*)];
	};

	void* pArena;
	int nCurrentSize;
	int nTotalSize;
	Rep_t* pRep;

	// @ida: #STR: "cl: CreateMove clamped invalid attack h" go down a bit and you will find it
	// @ida: #STR: "cl: CreateMove - Invalid player history [ %d, %d, %.3f ] f
	template <typename T>
	T* add(T* element)
	{
		// Define the function pointer correctly
		static auto add_to_rep_addr = reinterpret_cast<T*(__fastcall*)(RepeatedPtrField_t*, T*)>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 4C 8B E0 48 8B 44 24 ? 4C 8B CF")), 0x1));

		// Use the function pointer to call the function
		return add_to_rep_addr(this, element);
	}

	inline T*& operator[](int i)
	{
		return pRep->tElements[i];
	}
};

class CBasePB
{
private:
	void* VTable; // 0x0 VTABLE
public:
	std::uint32_t nHasBits; // 0x8
	std::uint64_t nCachedBits; // 0xC

	void SetBits(std::uint64_t nBits)
	{
		// @note: you don't need to check if the bits are already set as bitwise OR will not change the value if the bit is already set
		nCachedBits |= nBits;
	}
};

static_assert(sizeof(CBasePB) == 0x18);

class CMsgQAngle : public CBasePB
{
public:
	QAngle_t angValue; // 0x18
};

static_assert(sizeof(CMsgQAngle) == 0x28);

class CMsgVector : public CBasePB
{
public:
	Vector4D_t vecValue; // 0x18
};

static_assert(sizeof(CMsgVector) == 0x28);

class CCSGOInterpolationInfoPB : public CBasePB
{
public:
	float flFraction; // 0x18
	int nSrcTick; // 0x1C
	int nDstTick; // 0x20
};

static_assert(sizeof(CCSGOInterpolationInfoPB) == 0x28);

class CCSGOInterpolationInfoPB_CL : public CBasePB
{
public:
	float flFraction; // 0x18
};

static_assert(sizeof(CCSGOInterpolationInfoPB_CL) == 0x20);

class CCSGOInputHistoryEntryPB : public CBasePB
{
public:
	class CMsgQAngle* pViewAngles; //0x0018
	class CCSGOInterpolationInfoPB_CL* cl_interp; //0x0020
	class CCSGOInterpolationInfoPB* sv_interp0; //0x0028
	class CCSGOInterpolationInfoPB* sv_interp1; //0x0030
	class CCSGOInterpolationInfoPB* player_interp; //0x0038
	class CMsgVector* pShootPosition; //0x0040
	class CMsgVector* pTargetHeadPositionCheck; //0x0048
	class CMsgVector* pTargetAbsPositionCheck; //0x0050
	class CMsgQAngle* pTargetAngPositionCheck; //0x0058
	int32_t nRenderTickCount; //0x0060
	float flRenderTickFraction; //0x0064
	int32_t nPlayerTickCount; //0x0068
	float flPlayerTickFraction; //0x006C
	int32_t nFrameNumber; //0x0070
	int32_t nTargetEntIndex; //0x0074

	CMsgQAngle* CreateQAngle()
	{
		using fOriginal = CMsgQAngle*(CS_FASTCALL*)(void*);
		auto oOriginal = reinterpret_cast<fOriginal>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 89 45 ? 49 8D 8D")), 0x1));
		return oOriginal(nullptr);
	}

	CMsgVector* CreateVector()
	{
		using fOriginal = CMsgVector*(CS_FASTCALL*)(void*);
		auto oOriginal = reinterpret_cast<fOriginal>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 89 45 ? F3 0F 10 06")), 0x1));
		return oOriginal(nullptr);
	}

	CCSGOInterpolationInfoPB_CL* CreateInterpCL()
	{
		using fOriginal = CCSGOInterpolationInfoPB_CL*(CS_FASTCALL*)(void*);
		auto oOriginal = reinterpret_cast<fOriginal>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 89 43 ? 48 8B 43 ? F3 0F 10 45")), 0x1));
		return oOriginal(nullptr);
	}

	CCSGOInterpolationInfoPB* CreateInterp()
	{
		using fOriginal = CCSGOInterpolationInfoPB*(CS_FASTCALL*)(void*);
		auto oOriginal = reinterpret_cast<fOriginal>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 89 43 ? 8B 45")), 0x1));
		return oOriginal(nullptr);
	}
};

static_assert(sizeof(CCSGOInputHistoryEntryPB) == 0x78);

struct CInButtonStatePB : CBasePB
{
	std::uint64_t nValue;
	std::uint64_t nValueChanged;
	std::uint64_t nValueScroll;
};

static_assert(sizeof(CInButtonStatePB) == 0x30);

struct CSubtickMoveStep : CBasePB
{
public:
	std::uint64_t nButton;
	bool bPressed;
	float flWhen;
	float flAnalogForwardDelta;
	float flAnalogLeftDelta;
};

static_assert(sizeof(CSubtickMoveStep) == 0x30);

class CBaseUserCmdPB : public CBasePB
{
public:
	RepeatedPtrField_t<CSubtickMoveStep> subtickMovesField;
	std::string* strMoveCrc;
	CInButtonStatePB* pInButtonState;
	CMsgQAngle* pViewAngles;
	std::int32_t nLegacyCommandNumber;
	std::int32_t nClientTick;
	float flForwardMove;
	float flSideMove;
	float flUpMove;
	std::int32_t nImpulse;
	std::int32_t nWeaponSelect;
	std::int32_t nRandomSeed;
	std::int32_t nMousedX;
	std::int32_t nMousedY;
	std::uint32_t nConsumedServerAngleChanges;
	std::int32_t nCmdFlags;
	std::uint32_t nPawnEntityHandle;

	CSubtickMoveStep* AddSubtickMove()
	{
		using fn_add_subtick_move_step = CSubtickMoveStep* (__fastcall*)(void*);
		static fn_add_subtick_move_step fn_create_new_subtick_move_step = reinterpret_cast<fn_add_subtick_move_step>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8B D0 48 8D 4F 18 E8 ? ? ? ? 48 8B D0")), 0x1));

		if (subtickMovesField.pRep && subtickMovesField.nCurrentSize < subtickMovesField.pRep->nAllocatedSize)
			return subtickMovesField.pRep->tElements[subtickMovesField.nCurrentSize++];

		CSubtickMoveStep* subtick = fn_create_new_subtick_move_step(nullptr);
		subtickMovesField.add(subtick);

		return subtick;
	}

	void WriteButtonEvent(std::uint64_t button, bool pressed, float when) noexcept
	{
		auto subtick = this->AddSubtickMove();
		subtick->nButton = button;
		subtick->bPressed = pressed;
		subtick->flWhen = when;
		subtick->SetBits(ESubtickMoveStepBits::MOVESTEP_BITS_BUTTON);
		subtick->SetBits(ESubtickMoveStepBits::MOVESTEP_BITS_PRESSED);
		subtick->SetBits(ESubtickMoveStepBits::MOVESTEP_BITS_WHEN);
	}

	void RemoveButtonEvent(std::uint64_t button) noexcept
	{
		if (subtickMovesField.pRep)
		{
			for (int i{}; i < subtickMovesField.nCurrentSize; i++)
				if (auto step_move = subtickMovesField[i]; step_move && step_move->nButton == button)
					step_move->nButton = 0;
		}
	}

	void PressButton(std::uint64_t button) noexcept
	{
		float frac = I::GlobalVars->flTickFraction1;
		this->WriteButtonEvent(button, true, frac);
		this->WriteButtonEvent(button, false, frac + 0.001f);
	}

	int CalculateCmdCRCSize()
	{
		return MEM::CallVFunc<int, 7U>(this);
	}
};

static_assert(sizeof(CBaseUserCmdPB) == 0x80);

class CCSGOUserCmdPB
{
public:
	std::uint32_t nHasBits;
	std::uint64_t nCachedSize;
	RepeatedPtrField_t<CCSGOInputHistoryEntryPB> inputHistoryField;
	CBaseUserCmdPB* pBaseCmd;
	bool bLeftHandDesired;
	std::int32_t nAttack3StartHistoryIndex;
	std::int32_t nAttack1StartHistoryIndex;
	std::int32_t nAttack2StartHistoryIndex;

	// @note: this function is used to check if the bits are set and set them if they are not
	void CheckAndSetBits(std::uint32_t nBits)
	{
		if (!(nHasBits & nBits))
			nHasBits |= nBits;
	}
};
static_assert(sizeof(CCSGOUserCmdPB) == 0x40);

struct CInButtonState
{
public:
	MEM_PAD(0x8); // 0x0 VTABLE
	std::uint64_t nValue; // 0x8
	std::uint64_t nValueChanged; // 0x10
	std::uint64_t nValueScroll; // 0x18
};
static_assert(sizeof(CInButtonState) == 0x20);

class CUserCmd
{
private:
	void* VTable;

public:
	MEM_PAD(0x10); // TODO: find out what this is, added 14.08.2024
	CCSGOUserCmdPB csgoUserCmd; // 0x18
	CInButtonState nButtons; // 0x58
	MEM_PAD(0x10);
	bool bHasBeenPredicted;
	MEM_PAD(0xF);

	CCSGOInputHistoryEntryPB* GetInputHistoryEntry(int nIndex)
	{
		if (nIndex >= csgoUserCmd.inputHistoryField.pRep->nAllocatedSize || nIndex >= csgoUserCmd.inputHistoryField.nCurrentSize)
			return nullptr;

		return csgoUserCmd.inputHistoryField.pRep->tElements[nIndex];
	}

	CCSGOInputHistoryEntryPB* AddInputHistoryEntry()
	{
		using fOriginal = CCSGOInputHistoryEntryPB*(CS_FASTCALL*)(void*);
		auto oOriginal = reinterpret_cast<fOriginal>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 48 8B D0 49 8B CE E8 ? ? ? ? 4C 8B E0")), 0x1));
		return oOriginal(nullptr);
	}

	void ClearInputHistory()
	{
		for (int i = 0; i < csgoUserCmd.inputHistoryField.pRep->nAllocatedSize; i++)
		{
			I::MemAlloc->Free(csgoUserCmd.inputHistoryField.pRep->tElements[i]);
			csgoUserCmd.inputHistoryField.pRep->tElements[i] = nullptr;
		}
		csgoUserCmd.inputHistoryField.nCurrentSize = 0;
		csgoUserCmd.inputHistoryField.nTotalSize = 0;
		csgoUserCmd.inputHistoryField.pRep->nAllocatedSize = 0;
	}

	void SetSubTickAngle(const QAngle_t& angView)
	{
		for (int i = 0; i < this->csgoUserCmd.inputHistoryField.pRep->nAllocatedSize; i++)
		{
			CCSGOInputHistoryEntryPB* pInputEntry = this->GetInputHistoryEntry(i);
			if (!pInputEntry || !pInputEntry->pViewAngles)
				continue;

			pInputEntry->pViewAngles->angValue = angView;
			pInputEntry->SetBits(EInputHistoryBits::INPUT_HISTORY_BITS_VIEWANGLES);
		}
	}
};
static_assert(sizeof(CUserCmd) == 0x98);
