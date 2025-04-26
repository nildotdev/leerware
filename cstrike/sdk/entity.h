#pragma once

// @test: using interfaces in the header | not critical but could blow up someday with thousands of errors or affect to compilation time etc
// used: cgameentitysystem, ischemasystem
#include "../core/interfaces.h"
#include "interfaces/igameresourceservice.h"
#include "interfaces/ischemasystem.h"

// used: schema field
#include "../core/schema.h"

// used: l_print
#include "../utilities/log.h"
// used: vector_t
#include "datatypes/vector.h"
// used: qangle_t
#include "datatypes/qangle.h"
// used: ctransform
#include "datatypes/transform.h"
// used: cstronghandle
#include "datatypes/stronghandle.h"

// used: cbasehandle
#include "entity_handle.h"
// used: game's definitions
#include "const.h"
// used: entity vdata
#include "vdata.h"

enum EHitgroups : int
{
	HITGROUP_INVALID = -1,
	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD,
	HITGROUP_CHEST,
	HITGROUP_STOMACH,
	HITGROUP_LEFTARM,
	HITGROUP_RIGHTARM,
	HITGROUP_LEFTLEG,
	HITGROUP_RIGHTLEG,
	HITGROUP_NECK,
	HITGROUP_UNUSED,
	HITGROUP_GEAR,
	HITGROUP_SPECIAL,
	HITGROUP_COUNT,
};

enum EBoneFlags : unsigned int
{
	FLAG_NO_BONE_FLAGS = 0x0,
	FLAG_BONEFLEXDRIVER = 0x4,
	FLAG_CLOTH = 0x8,
	FLAG_PHYSICS = 0x10,
	FLAG_ATTACHMENT = 0x20,
	FLAG_ANIMATION = 0x40,
	FLAG_MESH = 0x80,
	FLAG_HITBOX = 0x100,
	FLAG_BONE_USED_BY_VERTEX_LOD0 = 0x400,
	FLAG_BONE_USED_BY_VERTEX_LOD1 = 0x800,
	FLAG_BONE_USED_BY_VERTEX_LOD2 = 0x1000,
	FLAG_BONE_USED_BY_VERTEX_LOD3 = 0x2000,
	FLAG_BONE_USED_BY_VERTEX_LOD4 = 0x4000,
	FLAG_BONE_USED_BY_VERTEX_LOD5 = 0x8000,
	FLAG_BONE_USED_BY_VERTEX_LOD6 = 0x10000,
	FLAG_BONE_USED_BY_VERTEX_LOD7 = 0x20000,
	FLAG_BONE_MERGE_READ = 0x40000,
	FLAG_BONE_MERGE_WRITE = 0x80000,
	FLAG_ALL_BONE_FLAGS = 0xfffff,
	BLEND_PREALIGNED = 0x100000,
	FLAG_RIGIDLENGTH = 0x200000,
	FLAG_PROCEDURAL = 0x400000,
};

enum EHitboxes : unsigned short
{
	HITBOX_HEAD = 0,
	// Not the same as head. Will deal way less damage.
	HITBOX_NECK,
	// Stomach.
	HITBOX_PELVIS,
	HITBOX_PELVIS1,
	HITBOX_STOMACH,
	// Chest.
	HITBOX_CHEST,
	HITBOX_CHEST1,
	// Legs.
	HITBOX_LEFTUPPERLEG,
	HITBOX_RIGHTUPPERLEG,
	HITBOX_LEFTLOWERLEG,
	HITBOX_RIGHTLOWERLEG,
	HITBOX_LEFTTOE,
	HITBOX_RIGHTTOE,
	// Arms.
	HITBOX_LEFTHAND,
	HITBOX_RIGHTHAND,
	HITBOX_LEFTUPPERARM,
	HITBOX_LEFTLOWERARM,
	HITBOX_RIGHTUPPERARM,
	HITBOX_RIGHTLOWERARM
};

using GameTime_t = std::float_t;
using GameTick_t = std::int32_t;

class CEntityInstance;

class CEntityIdentity
{
public:
	CS_CLASS_NO_INITIALIZER(CEntityIdentity);

	// @note: handle index is not entity index
	SCHEMA_ADD_OFFSET(std::uint32_t, GetIndex, 0x10);
	SCHEMA_ADD_FIELD(const char*, GetDesignerName, "CEntityIdentity->m_designerName");
	SCHEMA_ADD_FIELD(std::uint32_t, GetFlags, "CEntityIdentity->m_flags");

	[[nodiscard]] bool IsValid()
	{
		return GetIndex() != INVALID_EHANDLE_INDEX;
	}

	[[nodiscard]] int GetEntryIndex()
	{
		if (!IsValid())
			return ENT_ENTRY_MASK;
		
		return GetIndex() & ENT_ENTRY_MASK;
	}

	[[nodiscard]] int GetSerialNumber()
	{
		return GetIndex() >> NUM_SERIAL_NUM_SHIFT_BITS;
	}

	CEntityInstance* pInstance; // 0x00
};

class CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER(CEntityInstance);

	void GetSchemaClassInfo(SchemaClassInfoData_t** pReturn)
	{
		return MEM::CallVFunc<void, 38U>(this, pReturn);
	}

	[[nodiscard]] CBaseHandle GetRefEHandle()
	{
		CEntityIdentity* pIdentity = GetIdentity();
		if (pIdentity == nullptr)
			return CBaseHandle();

		return CBaseHandle(pIdentity->GetEntryIndex(), pIdentity->GetSerialNumber() - (pIdentity->GetFlags() & 1));
	}

	SCHEMA_ADD_FIELD(CEntityIdentity*, GetIdentity, "CEntityInstance->m_pEntity");
};

class CCollisionProperty
{
public:
	std::uint16_t CollisionMask()
	{
		return *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uintptr_t>(this) + 0x38);
	}

	SCHEMA_ADD_FIELD(Vector_t, GetMins, "CCollisionProperty->m_vecMins");
	SCHEMA_ADD_FIELD(Vector_t, GetMaxs, "CCollisionProperty->m_vecMaxs");

	SCHEMA_ADD_FIELD(std::uint8_t, GetSolidFlags, "CCollisionProperty->m_usSolidFlags");
	SCHEMA_ADD_FIELD(std::uint8_t, GetCollisionGroup, "CCollisionProperty->m_CollisionGroup");
};

class CSkeletonInstance;
class CGameSceneNode
{
public:
	CS_CLASS_NO_INITIALIZER(CGameSceneNode);

	SCHEMA_ADD_FIELD(CTransform, GetNodeToWorld, "CGameSceneNode->m_nodeToWorld");
	SCHEMA_ADD_FIELD(CEntityInstance*, GetOwner, "CGameSceneNode->m_pOwner");

	SCHEMA_ADD_FIELD(CNetworkOriginCellCoordQuantizedVector, GetOrigin, "CGameSceneNode->m_vecOrigin");
	SCHEMA_ADD_FIELD(Vector_t, GetAbsOrigin, "CGameSceneNode->m_vecAbsOrigin");
	SCHEMA_ADD_FIELD(Vector_t, GetRenderOrigin, "CGameSceneNode->m_vRenderOrigin");

	SCHEMA_ADD_FIELD(QAngle_t, GetAngleRotation, "CGameSceneNode->m_angRotation");
	SCHEMA_ADD_FIELD(QAngle_t, GetAbsAngleRotation, "CGameSceneNode->m_angAbsRotation");

	SCHEMA_ADD_FIELD(bool, IsDormant, "CGameSceneNode->m_bDormant");

	CSkeletonInstance* GetSkeletonInstance()
	{
		return MEM::CallVFunc<CSkeletonInstance*, 8U>(this);
	}
};

class CHitBox
{
public:
	char* m_name; //0x0000
	char* m_sSurfaceProperty; //0x0008
	char* m_sBoneName; //0x0010
	Vector_t m_vMinBounds; //0x0018
	Vector_t m_vMaxBounds; //0x0024
	float m_flShapeRadius; //0x0030
	uint32_t m_BoneNameHash; //0x0034
	int32_t m_nGroupId; //0x0038
	uint8_t m_nShapeType; //0x003C
	bool m_bTranslationOnly; //0x003D
	void* m_CRC; //0x003E
	void* m_cRenderColor; //0x0046
	uint16_t m_nHitboxIndex; //0x004E
	char pad_0050[30]; //0x0050
};
static_assert(sizeof(CHitBox) == 0x70);

class CHitBoxSet
{
public:
	CS_CLASS_NO_INITIALIZER(CHitBoxSet);

	SCHEMA_ADD_FIELD(const char*, GetName, "CHitBoxSet->m_name");
	SCHEMA_ADD_FIELD(unsigned int, GetNameHash, "CHitBoxSet->m_nNameHash");
	SCHEMA_ADD_FIELD(CUtlVectorCS2<CHitBox>, GetHitboxes, "CHitBoxSet->m_HitBoxes");
	SCHEMA_ADD_FIELD(const char*, GetSourceFilename, "CHitBoxSet->m_SourceFilename");
};

class C_BaseEntity : public CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseEntity);

	[[nodiscard]] bool IsBasePlayerController()
	{
		SchemaClassInfoData_t* pClassInfo;
		GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			return false;

		return FNV1A::Hash(pClassInfo->szName) == FNV1A::HashConst("C_CSPlayerController");
	}

	[[nodiscard]] bool IsWeapon()
	{
		static SchemaClassInfoData_t* pWeaponBaseClass = nullptr;
		if (pWeaponBaseClass == nullptr)
		I::SchemaSystem->FindTypeScopeForModule(CS_XOR("client.dll"))->FindDeclaredClass(&pWeaponBaseClass, CS_XOR("C_CSWeaponBase"));


		SchemaClassInfoData_t* pClassInfo;
		GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			return false;

		return (pClassInfo->InheritsFrom(pWeaponBaseClass));
	}

	[[nodiscard]] CHitBoxSet* GetHitboxSet(int i)
	{
		using fGetHitboxSet = CHitBoxSet*(CS_FASTCALL*)(void*, int);
		/*
		XREF Signature #1 @ 7FF93E5B60DB: E8 ? ? ? ? 4C 8B F8 48 85 C0 0F 84 ? ? ? ? 44 39 70
		XREF Signature #2 @ 7FF93E5B642A: E8 ? ? ? ? 48 8B F0 48 85 C0 0F 84 ? ? ? ? 44 39 70
		*/
		const auto oGetHitboxSet = reinterpret_cast<fGetHitboxSet>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 48 89 74 24 ? 57 48 81 EC ? ? ? ? 8B DA 48 8B F9 E8 ? ? ? ? 48 8B F0")));
#ifdef CS_PARANOID
		CS_ASSERT(oGetHitboxSet != nullptr);
#endif
		return oGetHitboxSet(this, i);
	}

	[[nodiscard]] int HitboxToWorldTransform(CHitBoxSet* hitboxSet, CTransform* outTransform)
	{
		using fHitboxToWorldTransform = int(CS_FASTCALL*)(void*, CHitBoxSet*, CTransform*, int);
		/*
		XREF Signature #1 @ 7FF93E5B6120: E8 ? ? ? ? C7 06
		XREF Signature #2 @ 7FF93E5B6459: E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? C7 07
		XREF Signature #3 @ 7FF93E7E84A5: E8 ? ? ? ? 44 8B F0 85 C0 7E ? 48 8B 9D
		XREF Signature #4 @ 7FF93E7E86D7: E8 ? ? ? ? 44 8B F8 85 C0 7E ? 48 8B 9D
		*/
		const auto oHitboxToWorldTransform = reinterpret_cast<fHitboxToWorldTransform>(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 55 57 41 54 41 56 41 57 48 83 EC")));
#ifdef CS_PARANOID
		CS_ASSERT(oHitboxToWorldTransform != nullptr);
#endif
		return oHitboxToWorldTransform(this, hitboxSet, outTransform, 1024);
	}

	static C_BaseEntity* GetLocalPlayer();

	// get entity origin on scene
	[[nodiscard]] const Vector_t& GetSceneOrigin();

	SCHEMA_ADD_FIELD(CGameSceneNode*, GetGameSceneNode, "C_BaseEntity->m_pGameSceneNode");
	SCHEMA_ADD_FIELD(CCollisionProperty*, GetCollision, "C_BaseEntity->m_pCollision");
	SCHEMA_ADD_FIELD(std::uint8_t, GetTeam, "C_BaseEntity->m_iTeamNum");
	SCHEMA_ADD_FIELD(CBaseHandle, GetOwnerHandle, "C_BaseEntity->m_hOwnerEntity");
	SCHEMA_ADD_FIELD(Vector_t, GetBaseVelocity, "C_BaseEntity->m_vecBaseVelocity");
	SCHEMA_ADD_FIELD(Vector_t, GetAbsVelocity, "C_BaseEntity->m_vecAbsVelocity");
	SCHEMA_ADD_FIELD(Vector_t, GetVecVelocity, "C_BaseEntity->m_vecVelocity");
	SCHEMA_ADD_FIELD(bool, IsTakingDamage, "C_BaseEntity->m_bTakesDamage");
	SCHEMA_ADD_FIELD(std::uint32_t, GetFlags, "C_BaseEntity->m_fFlags");
	SCHEMA_ADD_FIELD(std::int32_t, GetEflags, "C_BaseEntity->m_iEFlags");
	SCHEMA_ADD_FIELD(std::uint8_t, GetMoveType, "C_BaseEntity->m_nActualMoveType"); // m_nActualMoveType returns CSGO style movetype, m_nMoveType returns bitwise shifted move type
	SCHEMA_ADD_FIELD(std::uint8_t, GetLifeState, "C_BaseEntity->m_lifeState");
	SCHEMA_ADD_FIELD(std::int32_t, GetHealth, "C_BaseEntity->m_iHealth");
	SCHEMA_ADD_FIELD(std::int32_t, GetMaxHealth, "C_BaseEntity->m_iMaxHealth");
	SCHEMA_ADD_FIELD(std::float_t, GetSimulationTime, "C_BaseEntity->m_flSimulationTime");
	SCHEMA_ADD_FIELD(float, GetWaterLevel, "C_BaseEntity->m_flWaterLevel");
	SCHEMA_ADD_FIELD_OFFSET(void*, GetVData, "C_BaseEntity->m_nSubclassID", 0x8);
};

class CGlowProperty;

class C_BaseModelEntity : public C_BaseEntity
{
public:
	SCHEMA_ADD_FIELD(CCollisionProperty, GetCollisionInstance, "C_BaseModelEntity->m_Collision");
	//SCHEMA_ADD_FIELD(CGlowProperty, GetGlowProperty, "C_BaseModelEntity->m_Glow");
	SCHEMA_ADD_FIELD(Vector_t, GetViewOffset, "C_BaseModelEntity->m_vecViewOffset");
	SCHEMA_ADD_FIELD(GameTime_t, GetCreationTime, "C_BaseModelEntity->m_flCreateTime");
	SCHEMA_ADD_FIELD(GameTick_t, GetCreationTick, "C_BaseModelEntity->m_nCreationTick");
	SCHEMA_ADD_FIELD(CBaseHandle, GetMoveParent, "C_BaseModelEntity->m_hOldMoveParent");
	SCHEMA_ADD_FIELD(std::float_t, GetAnimTime, "C_BaseModelEntity->m_flAnimTime");
};

class CPlayer_ItemServices
{
public:
	CS_CLASS_NO_INITIALIZER(CPlayer_ItemServices);

	SCHEMA_ADD_FIELD(bool, m_bHasDefuser, "CCSPlayer_ItemServices->m_bHasDefuser");
	SCHEMA_ADD_FIELD(bool, m_bHasHelmet, "CCSPlayer_ItemServices->m_bHasHelmet");
	SCHEMA_ADD_FIELD(bool, m_bHasHeavyArmor, "CCSPlayer_ItemServices->m_bHasHeavyArmor");
};

class CPlayer_CameraServices
{
public:
	CS_CLASS_NO_INITIALIZER(CPlayer_CameraServices);

	SCHEMA_ADD_FIELD(CBaseHandle, GetViewEntity, "CPlayer_CameraServices->m_hViewEntity");
};

class CCSPlayerBase_CameraServices : public CPlayer_CameraServices
{
public:
	CS_CLASS_NO_INITIALIZER(CCSPlayerBase_CameraServices);

	SCHEMA_ADD_FIELD(uint32_t, GetFOV, "CCSPlayerBase_CameraServices->m_iFOV");
};

class CPlayer_WeaponServices
{
public:
	SCHEMA_ADD_FIELD(CBaseHandle, GetActiveWeapon, "CPlayer_WeaponServices->m_hActiveWeapon");
	SCHEMA_ADD_FIELD(GameTime_t, GetNextAttack, "CCSPlayer_WeaponServices->m_flNextAttack");
};

class CCSPlayer_WeaponServices : public CPlayer_WeaponServices
{
public:
	SCHEMA_ADD_FIELD(GameTime_t, GetNextAttack, "CCSPlayer_WeaponServices->m_flNextAttack");
}; 

class CUserCmd;
class CPlayer_MovementServices
{
public:
	CS_CLASS_NO_INITIALIZER(CPlayer_MovementServices);

	SCHEMA_ADD_FIELD(float, GetMaxSpeed, "CPlayer_MovementServices->m_flMaxspeed");
	SCHEMA_ADD_FIELD(float, GetForwardMove, "CPlayer_MovementServices->m_flForwardMove");
	SCHEMA_ADD_FIELD(float, GetLeftMove, "CPlayer_MovementServices->m_flLeftMove");
	SCHEMA_ADD_FIELD(float, GetUpMove, "CPlayer_MovementServices->m_flUpMove");
	SCHEMA_ADD_OFFSET(float, GetSurfaceFriction, 0x1FC);

	void SetPredictionCommand()//(CUserCmd* pCmd)
	{
		MEM::CallVFunc<void*, 34U>(this);
	}

	void ResetPredictionCommand()
	{
		MEM::CallVFunc<void*, 35U>(this);
	}
};

class CPlayer_MovementServices_Humanoid : public CPlayer_MovementServices
{
public:
	CS_CLASS_NO_INITIALIZER(CPlayer_MovementServices_Humanoid);

	SCHEMA_ADD_FIELD(float, GetStepSoundTime, "CPlayer_MovementServices_Humanoid->m_flStepSoundTime");
	SCHEMA_ADD_FIELD(float, GetFallVelocity, "CPlayer_MovementServices_Humanoid->m_flFallVelocity");
	SCHEMA_ADD_FIELD(bool, IsCrouching, "CPlayer_MovementServices_Humanoid->m_bInCrouch");
	SCHEMA_ADD_FIELD(unsigned int, GetCrouchState, "CPlayer_MovementServices_Humanoid->m_nCrouchState");
	SCHEMA_ADD_FIELD(bool, IsDucked, "CPlayer_MovementServices_Humanoid->m_bDucked");
	SCHEMA_ADD_FIELD(bool, IsDucking, "CPlayer_MovementServices_Humanoid->m_bDucking");
	SCHEMA_ADD_FIELD(bool, IsInDuckJump, "CPlayer_MovementServices_Humanoid->m_bInDuckJump");
	SCHEMA_ADD_FIELD(Vector_t, GetGroundNormal, "CPlayer_MovementServices_Humanoid->m_groundNormal");
	SCHEMA_ADD_FIELD(float, GetSurfaceFriction, "CPlayer_MovementServices_Humanoid->m_flSurfaceFriction");
};

class CCSPlayer_MovementServices : public CPlayer_MovementServices_Humanoid
{
public:
	CS_CLASS_NO_INITIALIZER(CCSPlayer_MovementServices);
};

class C_BasePlayerPawn : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_BasePlayerPawn);

	SCHEMA_ADD_FIELD(CBaseHandle, GetControllerHandle, "C_BasePlayerPawn->m_hController");
	SCHEMA_ADD_FIELD(CCSPlayer_WeaponServices*, GetWeaponServices, "C_BasePlayerPawn->m_pWeaponServices");
	SCHEMA_ADD_FIELD(CPlayer_ItemServices*, GetItemServices, "C_BasePlayerPawn->m_pItemServices");
	SCHEMA_ADD_FIELD(CPlayer_CameraServices*, GetCameraServices, "C_BasePlayerPawn->m_pCameraServices");
	SCHEMA_ADD_FIELD(CPlayer_MovementServices*, GetMovementServices, "C_BasePlayerPawn->m_pMovementServices");

	[[nodiscard]] Vector_t GetEyePosition()
	{
		Vector_t vecEyePosition = Vector_t(0.0f, 0.0f, 0.0f);
		// Credit: https://www.unknowncheats.me/forum/4258133-post6228.html
		MEM::CallVFunc<void, 169U>(this, &vecEyePosition);
		return vecEyePosition;
	}
};

class CCSPlayer_ViewModelServices;

class C_CSPlayerPawnBase : public C_BasePlayerPawn
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSPlayerPawnBase);

	SCHEMA_ADD_FIELD(CCSPlayer_ViewModelServices*, GetViewModelServices, "C_CSPlayerPawnBase->m_pViewModelServices");
	SCHEMA_ADD_FIELD(float, GetLowerBodyYawTarget, "C_CSPlayerPawnBase->m_flLowerBodyYawTarget");
	SCHEMA_ADD_FIELD(float, GetFlashMaxAlpha, "C_CSPlayerPawnBase->m_flFlashMaxAlpha");
	SCHEMA_ADD_FIELD(float, GetFlashDuration, "C_CSPlayerPawnBase->m_flFlashDuration");
	SCHEMA_ADD_FIELD(Vector_t, GetLastSmokeOverlayColor, "C_CSPlayerPawnBase->m_vLastSmokeOverlayColor");
	SCHEMA_ADD_FIELD(int, GetSurvivalTeam, "C_CSPlayerPawnBase->m_nSurvivalTeam"); // danger zone
	SCHEMA_ADD_FIELD(QAngle_t, GetEyeAngles, "C_CSPlayerPawnBase->m_angEyeAngles");
};

// forward decleration
class C_CSWeaponBase;
class C_CSWeaponBaseGun;
class C_BasePlayerWeapon;

class C_CSPlayerPawn : public C_CSPlayerPawnBase
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSPlayerPawn);

	[[nodiscard]] bool IsOtherEnemy(C_CSPlayerPawn* pOther);
	[[nodiscard]] int GetAssociatedTeam();
	[[nodiscard]] bool CanAttack();
	[[nodiscard]] std::uint32_t GetOwnerHandleIndex();
	[[nodiscard]] std::uint16_t GetCollisionMask();
	[[nodiscard]] bool HasArmor(const int hitgroup);
	[[nodiscard]] C_CSWeaponBaseGun* GetCurrentWeapon();

	SCHEMA_ADD_FIELD(bool, IsScoped, "C_CSPlayerPawn->m_bIsScoped");
	SCHEMA_ADD_FIELD(bool, IsDefusing, "C_CSPlayerPawn->m_bIsDefusing");
	SCHEMA_ADD_FIELD(bool, IsGrabbingHostage, "C_CSPlayerPawn->m_bIsGrabbingHostage");
	SCHEMA_ADD_FIELD(bool, IsWaitForNoAttack, "C_CSPlayerPawn->m_bWaitForNoAttack");
	SCHEMA_ADD_FIELD(bool, WasOnGroundLastTick, "C_CSPlayerPawn->m_bOnGroundLastTick");
	SCHEMA_ADD_FIELD(int, GetShotsFired, "C_CSPlayerPawn->m_iShotsFired");
	SCHEMA_ADD_FIELD(GameTime_t, GetHealthshotExpirationTime, "C_CSPlayerPawn->m_flHealthShotBoostExpirationTime");
	SCHEMA_ADD_FIELD(std::int32_t, GetArmorValue, "C_CSPlayerPawn->m_ArmorValue");
	SCHEMA_ADD_FIELD(QAngle_t, GetAimPunchAngle, "C_CSPlayerPawn->m_aimPunchAngle");
	SCHEMA_ADD_FIELD(CUtlVectorCS2<QAngle_t>, GetAimPunchCache, "C_CSPlayerPawn->m_aimPunchCache");
};

class CBasePlayerController : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(CBasePlayerController);

	SCHEMA_ADD_FIELD(std::uint64_t, GetSteamId, "CBasePlayerController->m_steamID");
	SCHEMA_ADD_FIELD(std::uint32_t, GetTickBase, "CBasePlayerController->m_nTickBase");
	SCHEMA_ADD_FIELD(CBaseHandle, GetPawnHandle, "CBasePlayerController->m_hPawn");
	SCHEMA_ADD_FIELD(bool, IsLocalPlayerController, "CBasePlayerController->m_bIsLocalPlayerController");
	SCHEMA_ADD_FIELD_OFFSET(CUserCmd*, GetCurrentCommand, "CBasePlayerController->m_steamID", -0x8);
};

class CCSPlayerController : public CBasePlayerController
{
public:
	CS_CLASS_NO_INITIALIZER(CCSPlayerController);

	[[nodiscard]] static CCSPlayerController* GetLocalPlayerController();

	// @note: always get origin from pawn not controller
	[[nodiscard]] const Vector_t& GetPawnOrigin();

	[[nodiscard]] C_CSWeaponBase* GetPlayerWeapon();

	[[nodiscard]] bool IsThrowingGrenade();

	SCHEMA_ADD_FIELD(std::uint32_t, GetPing, "CCSPlayerController->m_iPing");
	SCHEMA_ADD_FIELD(const char*, GetPlayerName, "CCSPlayerController->m_sSanitizedPlayerName");
	SCHEMA_ADD_FIELD(std::int32_t, GetPawnHealth, "CCSPlayerController->m_iPawnHealth");
	SCHEMA_ADD_FIELD(std::int32_t, GetPawnArmor, "CCSPlayerController->m_iPawnArmor");
	SCHEMA_ADD_FIELD(bool, IsPawnHasDefuser, "CCSPlayerController->m_bPawnHasDefuser");
	SCHEMA_ADD_FIELD(bool, IsPawnHasHelmet, "CCSPlayerController->m_bPawnHasHelmet");
	SCHEMA_ADD_FIELD(bool, IsPawnAlive, "CCSPlayerController->m_bPawnIsAlive");
	SCHEMA_ADD_FIELD(CBaseHandle, GetPlayerPawnHandle, "CCSPlayerController->m_hPlayerPawn");
};

class C_FuncLadder : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_FuncLadder);

	SCHEMA_ADD_FIELD(bool, IsDisabled, "C_FuncLadder->m_bDisabled");
	SCHEMA_ADD_FIELD(Vector_t, GetMountTop, "C_FuncLadder->m_vecPlayerMountPositionTop");
	SCHEMA_ADD_FIELD(Vector_t, GetMountBottom, "C_FuncLadder->m_vecPlayerMountPositionBottom");
};

class CBaseAnimGraph : public C_BaseModelEntity
{
public:
	CS_CLASS_NO_INITIALIZER(CBaseAnimGraph);

	SCHEMA_ADD_FIELD(bool, IsClientRagdoll, "CBaseAnimGraph->m_bClientRagdoll");
};

class C_BaseFlex : public CBaseAnimGraph
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseFlex);
	/* not implemented */
};

class C_EconItemView
{
public:
	CS_CLASS_NO_INITIALIZER(C_EconItemView);

	SCHEMA_ADD_FIELD(std::uint16_t, GetItemDefinitionIndex, "C_EconItemView->m_iItemDefinitionIndex");
	SCHEMA_ADD_FIELD(std::uint64_t, GetItemID, "C_EconItemView->m_iItemID");
	SCHEMA_ADD_FIELD(std::uint32_t, GetItemIDHigh, "C_EconItemView->m_iItemIDHigh");
	SCHEMA_ADD_FIELD(std::uint32_t, GetItemIDLow, "C_EconItemView->m_iItemIDLow");
	SCHEMA_ADD_FIELD(std::uint32_t, GetAccountID, "C_EconItemView->m_iAccountID");
};

class CAttributeManager
{
public:
	CS_CLASS_NO_INITIALIZER(CAttributeManager);
	virtual ~CAttributeManager() = 0;
};
static_assert(sizeof(CAttributeManager) == 0x8);

class C_AttributeContainer : public CAttributeManager
{
public:
	CS_CLASS_NO_INITIALIZER(C_AttributeContainer);

	SCHEMA_ADD_PFIELD(C_EconItemView, GetItem, "C_AttributeContainer->m_Item");
};

class C_EconEntity : public C_BaseFlex
{
public:
	CS_CLASS_NO_INITIALIZER(C_EconEntity);

	SCHEMA_ADD_PFIELD(C_AttributeContainer, GetAttributeManager, "C_EconEntity->m_AttributeManager");
	SCHEMA_ADD_FIELD(std::uint32_t, GetOriginalOwnerXuidLow, "C_EconEntity->m_OriginalOwnerXuidLow");
	SCHEMA_ADD_FIELD(std::uint32_t, GetOriginalOwnerXuidHigh, "C_EconEntity->m_OriginalOwnerXuidHigh");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackPaintKit, "C_EconEntity->m_nFallbackPaintKit");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackSeed, "C_EconEntity->m_nFallbackSeed");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackWear, "C_EconEntity->m_flFallbackWear");
	SCHEMA_ADD_FIELD(std::int32_t, GetFallbackStatTrak, "C_EconEntity->m_nFallbackStatTrak");
	SCHEMA_ADD_FIELD(CBaseHandle, GetViewModelAttachmentHandle, "C_EconEntity->m_hViewmodelAttachment");
};

class C_EconWearable : public C_EconEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_EconWearable);

	SCHEMA_ADD_FIELD(std::int32_t, GetForceSkin, "C_EconWearable->m_nForceSkin");
	SCHEMA_ADD_FIELD(bool, IsAlwaysAllow, "C_EconWearable->m_bAlwaysAllow");
};

class C_BasePlayerWeapon : public C_EconEntity
{
public:
	CS_CLASS_NO_INITIALIZER(C_BasePlayerWeapon);

	SCHEMA_ADD_FIELD(GameTick_t, GetNextPrimaryAttackTick, "C_BasePlayerWeapon->m_nNextPrimaryAttackTick");
	SCHEMA_ADD_FIELD(float, GetNextPrimaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextPrimaryAttackTickRatio");
	SCHEMA_ADD_FIELD(GameTick_t, GetNextSecondaryAttackTick, "C_BasePlayerWeapon->m_nNextSecondaryAttackTick");
	SCHEMA_ADD_FIELD(float, GetNextSecondaryAttackTickRatio, "C_BasePlayerWeapon->m_flNextSecondaryAttackTickRatio");
	SCHEMA_ADD_FIELD(std::int32_t, GetClip1, "C_BasePlayerWeapon->m_iClip1");
	SCHEMA_ADD_FIELD(std::int32_t, GetClip2, "C_BasePlayerWeapon->m_iClip2");
};

class C_CSWeaponBase : public C_BasePlayerWeapon
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSWeaponBase);

	SCHEMA_ADD_FIELD(bool, IsInReload, "C_CSWeaponBase->m_bInReload");
	SCHEMA_ADD_FIELD(float, GetRecoilIndex, "C_CSWeaponBase->m_flRecoilIndex");
	SCHEMA_ADD_FIELD(float, GetWatOffset, "C_CSWeaponBase->m_flWatTickOffset");
	SCHEMA_ADD_FIELD(bool, IsBurstMode, "C_CSWeaponBase->m_bBurstMode");
	SCHEMA_ADD_FIELD(float, GetPostponeFireReadyFrac, "C_CSWeaponBase->m_flPostponeFireReadyFrac");

	CS_INLINE float GetSpread()
	{
		/*
		* These are important, so here are all the sigs:
		* 48 83 EC ? 48 63 91 - direct
		* No code XREFs ??? :(
		*/

		/*
		__m128 __fastcall GetSpread(__int64 a1)
		{
		  unsigned __int64 v1; // rdx
		  __int64 v2; // rax
		  __int128 v3; // xmm6

		  v1 = *(int *)(a1 + 6072);
		  v2 = *(_QWORD *)(a1 + 896);
		  if ( (v1 & 0x80000000) != 0i64 || v1 >= 2 )
			v3 = *(unsigned int *)(v2 + 3412);
		  else
			v3 = *(unsigned int *)(v2 + 4 * v1 + 3412);
		  sub_7FF93F16DD40(&unk_7FF93FB0DDB8, 0xFFFFFFFFi64);
		  return (__m128)v3;
		}
		*/

		/*
		.text:00007FF93E503C30
		.text:00007FF93E503C30 ; =============== S U B R O U T I N E =======================================
		.text:00007FF93E503C30
		.text:00007FF93E503C30
		.text:00007FF93E503C30 ; __m128 __fastcall GetSpread(__int64)
		.text:00007FF93E503C30 GetSpread       proc near               ; DATA XREF: .rdata:00007FF93F33F300↓o
		.text:00007FF93E503C30                                         ; .rdata:00007FF93F3401B0↓o ...
		.text:00007FF93E503C30
		.text:00007FF93E503C30 var_18          = xmmword ptr -18h
		.text:00007FF93E503C30
		.text:00007FF93E503C30                 sub     rsp, 38h
		.text:00007FF93E503C34                 movsxd  rdx, dword ptr [rcx+17B8h]
		.text:00007FF93E503C3B                 mov     rax, [rcx+380h]
		.text:00007FF93E503C42                 movaps  [rsp+38h+var_18], xmm6
		.text:00007FF93E503C47                 test    edx, edx
		.text:00007FF93E503C49                 js      short loc_7FF93E503C5C
		.text:00007FF93E503C4B                 cmp     rdx, 2
		.text:00007FF93E503C4F                 jnb     short loc_7FF93E503C5C
		.text:00007FF93E503C51                 movss   xmm6, dword ptr [rax+rdx*4+0D54h]
		.text:00007FF93E503C5A                 jmp     short loc_7FF93E503C64
		.text:00007FF93E503C5C ; ---------------------------------------------------------------------------
		.text:00007FF93E503C5C
		.text:00007FF93E503C5C loc_7FF93E503C5C:                       ; CODE XREF: GetSpread+19↑j
		.text:00007FF93E503C5C                                         ; GetSpread+1F↑j
		.text:00007FF93E503C5C                 movss   xmm6, dword ptr [rax+0D54h]
		.text:00007FF93E503C64
		.text:00007FF93E503C64 loc_7FF93E503C64:                       ; CODE XREF: GetSpread+2A↑j
		.text:00007FF93E503C64                 mov     edx, 0FFFFFFFFh
		.text:00007FF93E503C69                 lea     rcx, unk_7FF93FB0DDB8
		.text:00007FF93E503C70                 call    sub_7FF93F16DD40
		.text:00007FF93E503C75                 movaps  xmm0, xmm6
		.text:00007FF93E503C78                 movaps  xmm6, [rsp+38h+var_18]
		.text:00007FF93E503C7D                 add     rsp, 38h
		.text:00007FF93E503C81                 retn
		.text:00007FF93E503C81 GetSpread       endp
		.text:00007FF93E503C81
		.text:00007FF93E503C81 ; ---------------------------------------------------------------------------
		*/
		return MEM::CallVFunc<float, 364>(this);
	}

	CS_INLINE float GetInaccuracy()
	{
		/*
		* These are important, so here are all the sigs:
		* 48 89 5C 24 ? 55 56 57 48 81 EC ? ? ? ? 44 0F 29 84 24 - direct
		* No code XREFs ??? :(
		*/

		/*
		double __fastcall GetInaccuracy(float *a1, float *a2, float *a3)
		{
		  _BYTE *v6; // rdi
		  double result; // xmm0_8
		  _BYTE *v8; // rax
		  float v9; // xmm7_4
		  double v10; // xmm0_8
		  float v11; // xmm11_4
		  char v12; // r14
		  double v13; // xmm0_8
		  float v14; // xmm9_4
		  __int64 v15; // rax
		  double v16; // xmm12_8
		  int v17; // edx
		  int v18; // ecx
		  int v19; // r8d
		  int v20; // r9d
		  float v21; // xmm0_4
		  float v22; // xmm7_4
		  float *v23; // rax
		  float v24; // xmm6_4
		  float v25; // xmm10_4
		  float v26; // xmm0_4
		  float v27; // xmm9_4
		  float v28; // xmm1_4
		  float v29; // xmm6_4
		  int v30; // edi
		  _BYTE *v31; // rax
		  int v32; // [rsp+30h] [rbp-98h] BYREF
		  char v33[12]; // [rsp+34h] [rbp-94h] BYREF
		  int v34; // [rsp+E8h] [rbp+20h] BYREF

		  v6 = (_BYTE *)sub_7FF93E5C5AD0();
		  if ( !v6 )
			return 0.0;
		  if ( sub_7FF93E1339B0(&unk_7FF93FA59C60, 0xFFFFFFFFi64) > 0.0 )
		  {
			v34 = 1065353216;
			*(float *)&result = sub_7FF93E1339B0(&unk_7FF93FA59C60, 0xFFFFFFFFi64);
			v32 = LODWORD(result);
			sub_7FF93E129DD0(&v32, &v34);
			return result;
		  }
		  v8 = (_BYTE *)sub_7FF93F16DD40(&unk_7FF93FA59C70, 0xFFFFFFFFi64);
		  if ( !v8 )
			v8 = *(_BYTE **)(qword_7FF93FA59C78 + 8);
		  if ( *v8 )
			return 0.0;
		  (*(void (__fastcall **)(float *))(*(_QWORD *)a1 + 2840i64))(a1);
		  v9 = a1[1524];
		  sub_7FF93E175CC0(v6, v33);
		  v10 = sub_7FF93E143BA0();
		  v11 = *(float *)&v10;
		  v12 = v6[9080];
		  v13 = (*(double (__fastcall **)(float *, _QWORD))(*(_QWORD *)a1 + 2792i64))(a1, *((unsigned int *)a1 + 1518));
		  v14 = *(float *)&v13;
		  v15 = sub_7FF93E175CC0(v6, v33);
		  sub_7FF93E50C7A0(v15);
		  HIDWORD(v16) = 0;
		  sub_7FF93E181300(v18, v17, v19, v20, 1065353216);
		  if ( *(float *)&v13 <= 0.0 )
		  {
			v21 = 0.0;
		  }
		  else
		  {
			if ( !v12 )
			  *(float *)&v13 = V_powf();
			v21 = *(float *)&v13 * v14;
		  }
		  if ( a2 )
			*a2 = v21;
		  v22 = v9 + v21;
		  if ( !(*(__int64 (__fastcall **)(_BYTE *))(*(_QWORD *)v6 + 1432i64))(v6) )
		  {
			v23 = (float *)sub_7FF93F16DD40(&unk_7FF93FA59C80, 0xFFFFFFFFi64);
			if ( !v23 )
			  v23 = *(float **)(qword_7FF93FA59C88 + 8);
			v24 = *v23 * *(float *)(*((_QWORD *)a1 + 112) + 3516i64);
			v25 = sub_7FF93E1339B0(&unk_7FF93FA59C80, 0xFFFFFFFFi64) * *(float *)(*((_QWORD *)a1 + 112) + 3520i64);
			v26 = sub_7FF93E1339B0(&unk_7FF93FAA27D0, 0xFFFFFFFFi64);
			if ( v26 < 0.0 )
			  v27 = sqrtf(v26);
			else
			  v27 = fsqrt(v26);
			if ( v11 < 0.0 )
			  v28 = sqrtf(v11);
			else
			  v28 = fsqrt(v11);
			if ( (float)(v27 * 0.25) == v27 )
			{
			  if ( (float)(v28 - v27) >= 0.0 )
				v25 = v24;
			}
			else
			{
			  v25 = (float)((float)((float)(v28 - (float)(v27 * 0.25)) * (float)(v24 - v25)) / (float)(v27 - (float)(v27 * 0.25)))
				  + v25;
			}
			if ( v25 >= 0.0 )
			  v29 = fminf(v24 + v24, v25);
			else
			  v29 = 0.0;
			if ( a3 )
			  *a3 = v29;
			v22 = v22 + v29;
		  }
		  v30 = *(_DWORD *)(*((_QWORD *)a1 + 112) + 3392i64);
		  v31 = (_BYTE *)sub_7FF93F16DD40(&unk_7FF93FB0DDB8, 0xFFFFFFFFi64);
		  if ( !v31 )
			v31 = *(_BYTE **)(qword_7FF93FB0DDC0 + 8);
		  if ( *v31 )
		  {
			if ( v30 > 1 )
			  v22 = v22 + 0.0;
		  }
		  *(float *)&v16 = fminf(1.0, sub_7FF93E503C90(a1) + (float)(v22 + a1[1523]));
		  return v16;
		}
		*/

		/*
		.text:00007FF93E502190
		.text:00007FF93E502190 ; =============== S U B R O U T I N E =======================================
		.text:00007FF93E502190
		.text:00007FF93E502190
		.text:00007FF93E502190 ; double __fastcall GetInaccuracy(float *, float *, float *)
		.text:00007FF93E502190 GetInaccuracy   proc near               ; DATA XREF: .rdata:00007FF93F33F478↓o
		.text:00007FF93E502190                                         ; .rdata:00007FF93F340328↓o ...
		.text:00007FF93E502190
		.text:00007FF93E502190 var_A8          = dword ptr -0A8h
		.text:00007FF93E502190 var_98          = dword ptr -98h
		.text:00007FF93E502190 var_94          = byte ptr -94h
		.text:00007FF93E502190 var_88          = xmmword ptr -88h
		.text:00007FF93E502190 var_78          = xmmword ptr -78h
		.text:00007FF93E502190 var_68          = xmmword ptr -68h
		.text:00007FF93E502190 var_58          = xmmword ptr -58h
		.text:00007FF93E502190 var_48          = xmmword ptr -48h
		.text:00007FF93E502190 var_38          = xmmword ptr -38h
		.text:00007FF93E502190 var_28          = xmmword ptr -28h
		.text:00007FF93E502190 var_18          = byte ptr -18h
		.text:00007FF93E502190 arg_0           = qword ptr  8
		.text:00007FF93E502190 arg_8           = qword ptr  10h
		.text:00007FF93E502190 arg_18          = dword ptr  20h
		.text:00007FF93E502190
		.text:00007FF93E502190                 mov     [rsp+arg_8], rbx
		.text:00007FF93E502195                 push    rbp
		.text:00007FF93E502196                 push    rsi
		.text:00007FF93E502197                 push    rdi
		.text:00007FF93E502198                 sub     rsp, 0B0h
		.text:00007FF93E50219F                 movaps  [rsp+0C8h+var_48], xmm8
		.text:00007FF93E5021A8                 mov     rbp, r8
		.text:00007FF93E5021AB                 mov     rsi, rdx
		.text:00007FF93E5021AE                 mov     rbx, rcx
		.text:00007FF93E5021B1                 call    sub_7FF93E5C5AD0
		.text:00007FF93E5021B6                 mov     rdi, rax
		.text:00007FF93E5021B9                 xorps   xmm8, xmm8
		.text:00007FF93E5021BD                 test    rax, rax
		.text:00007FF93E5021C0                 jz      loc_7FF93E5024E4
		.text:00007FF93E5021C6                 mov     edx, 0FFFFFFFFh
		.text:00007FF93E5021CB                 lea     rcx, unk_7FF93FA59C60
		.text:00007FF93E5021D2                 call    sub_7FF93E1339B0
		.text:00007FF93E5021D7                 comiss  xmm0, xmm8
		.text:00007FF93E5021DB                 mov     edx, 0FFFFFFFFh
		.text:00007FF93E5021E0                 jbe     short loc_7FF93E502216
		.text:00007FF93E5021E2                 lea     rcx, unk_7FF93FA59C60
		.text:00007FF93E5021E9                 mov     [rsp+0C8h+arg_18], 3F800000h
		.text:00007FF93E5021F4                 call    sub_7FF93E1339B0
		.text:00007FF93E5021F9                 lea     rdx, [rsp+0C8h+arg_18]
		.text:00007FF93E502201                 movss   [rsp+0C8h+var_98], xmm0
		.text:00007FF93E502207                 lea     rcx, [rsp+0C8h+var_98]
		.text:00007FF93E50220C                 call    sub_7FF93E129DD0
		.text:00007FF93E502211                 jmp     loc_7FF93E5024E7
		.text:00007FF93E502216 ; ---------------------------------------------------------------------------
		.text:00007FF93E502216
		.text:00007FF93E502216 loc_7FF93E502216:                       ; CODE XREF: GetInaccuracy+50↑j
		.text:00007FF93E502216                 lea     rcx, unk_7FF93FA59C70
		.text:00007FF93E50221D                 call    sub_7FF93F16DD40
		.text:00007FF93E502222                 test    rax, rax
		.text:00007FF93E502225                 jnz     short loc_7FF93E502232
		.text:00007FF93E502227                 mov     rax, cs:qword_7FF93FA59C78
		.text:00007FF93E50222E                 mov     rax, [rax+8]
		.text:00007FF93E502232
		.text:00007FF93E502232 loc_7FF93E502232:                       ; CODE XREF: GetInaccuracy+95↑j
		.text:00007FF93E502232                 cmp     byte ptr [rax], 0
		.text:00007FF93E502235                 jnz     loc_7FF93E5024E4
		.text:00007FF93E50223B                 mov     rax, [rbx]
		.text:00007FF93E50223E                 mov     rcx, rbx
		.text:00007FF93E502241
		.text:00007FF93E502241 loc_7FF93E502241:                       ; DATA XREF: .rdata:00007FF93F6BEAD8↓o
		.text:00007FF93E502241                                         ; .rdata:00007FF93F6BEAFC↓o ...
		.text:00007FF93E502241                 mov     [rsp+0C8h+arg_0], r14
		.text:00007FF93E502249                 movaps  [rsp+0C8h+var_28], xmm6
		.text:00007FF93E502251                 movaps  [rsp+0C8h+var_38], xmm7
		.text:00007FF93E502259                 movaps  [rsp+0C8h+var_58], xmm9
		.text:00007FF93E50225F                 movaps  [rsp+0C8h+var_78], xmm11
		.text:00007FF93E502265                 movaps  [rsp+0C8h+var_88], xmm12
		.text:00007FF93E50226B                 call    qword ptr [rax+0B18h]
		.text:00007FF93E502271                 movss   xmm7, dword ptr [rbx+17D0h]
		.text:00007FF93E502279                 lea     rdx, [rsp+0C8h+var_94]
		.text:00007FF93E50227E                 mov     rcx, rdi
		.text:00007FF93E502281                 movaps  xmm6, xmm0
		.text:00007FF93E502284                 call    sub_7FF93E175CC0
		.text:00007FF93E502289                 movss   xmm0, dword ptr [rax+8]
		.text:00007FF93E50228E                 call    sub_7FF93E143BA0
		.text:00007FF93E502293                 mov     rax, [rbx]
		.text:00007FF93E502296                 mov     rcx, rbx
		.text:00007FF93E502299                 mov     edx, [rbx+17B8h]
		.text:00007FF93E50229F                 movaps  xmm11, xmm0
		.text:00007FF93E5022A3                 movzx   r14d, byte ptr [rdi+2378h]
		.text:00007FF93E5022AB                 call    qword ptr [rax+0AE8h]
		.text:00007FF93E5022B1                 lea     rdx, [rsp+0C8h+var_94]
		.text:00007FF93E5022B6                 mov     rcx, rdi
		.text:00007FF93E5022B9                 movaps  xmm9, xmm0
		.text:00007FF93E5022BD                 call    sub_7FF93E175CC0
		.text:00007FF93E5022C2                 mov     rcx, rax
		.text:00007FF93E5022C5                 call    sub_7FF93E50C7A0
		.text:00007FF93E5022CA                 movss   xmm12, cs:Y
		.text:00007FF93E5022D3                 movaps  xmm2, xmm6
		.text:00007FF93E5022D6                 mulss   xmm6, cs:dword_7FF93F328210
		.text:00007FF93E5022DE                 xorps   xmm3, xmm3
		.text:00007FF93E5022E1                 mulss   xmm2, cs:dword_7FF93F3279D8
		.text:00007FF93E5022E9                 movaps  xmm1, xmm6
		.text:00007FF93E5022EC                 movss   [rsp+0C8h+var_A8], xmm12
		.text:00007FF93E5022F3                 call    sub_7FF93E181300
		.text:00007FF93E5022F8                 comiss  xmm0, xmm8
		.text:00007FF93E5022FC                 jbe     short loc_7FF93E502318
		.text:00007FF93E5022FE                 test    r14b, r14b
		.text:00007FF93E502301                 jnz     short loc_7FF93E502311
		.text:00007FF93E502303                 movss   xmm1, cs:dword_7FF93F2A0098
		.text:00007FF93E50230B                 call    cs:__imp_V_powf
		.text:00007FF93E502311
		.text:00007FF93E502311 loc_7FF93E502311:                       ; CODE XREF: GetInaccuracy+171↑j
		.text:00007FF93E502311                 mulss   xmm0, xmm9
		.text:00007FF93E502316                 jmp     short loc_7FF93E50231B
		.text:00007FF93E502318 ; ---------------------------------------------------------------------------
		.text:00007FF93E502318
		.text:00007FF93E502318 loc_7FF93E502318:                       ; CODE XREF: GetInaccuracy+16C↑j
		.text:00007FF93E502318                 xorps   xmm0, xmm0
		.text:00007FF93E50231B
		.text:00007FF93E50231B loc_7FF93E50231B:                       ; CODE XREF: GetInaccuracy+186↑j
		.text:00007FF93E50231B                 mov     r14, [rsp+0C8h+arg_0]
		.text:00007FF93E502323                 test    rsi, rsi
		.text:00007FF93E502326                 jz      short loc_7FF93E50232C
		.text:00007FF93E502328
		.text:00007FF93E502328 loc_7FF93E502328:                       ; DATA XREF: .rdata:00007FF93F6BEB10↓o
		.text:00007FF93E502328                                         ; .rdata:00007FF93F6BEB20↓o ...
		.text:00007FF93E502328                 movss   dword ptr [rsi], xmm0
		.text:00007FF93E50232C
		.text:00007FF93E50232C loc_7FF93E50232C:                       ; CODE XREF: GetInaccuracy+196↑j
		.text:00007FF93E50232C                 mov     rax, [rdi]
		.text:00007FF93E50232F                 addss   xmm7, xmm0
		.text:00007FF93E502333                 mov     rcx, rdi
		.text:00007FF93E502336                 call    qword ptr [rax+598h]
		.text:00007FF93E50233C                 test    rax, rax
		.text:00007FF93E50233F                 jnz     loc_7FF93E502466
		.text:00007FF93E502345                 mov     edx, 0FFFFFFFFh
		.text:00007FF93E50234A
		.text:00007FF93E50234A loc_7FF93E50234A:                       ; DATA XREF: .rdata:00007FF93F6BEB10↓o
		.text:00007FF93E50234A                                         ; .rdata:00007FF93F6BEB20↓o ...
		.text:00007FF93E50234A                 movaps  [rsp+0C8h+var_68], xmm10
		.text:00007FF93E502350                 lea     rcx, unk_7FF93FA59C80
		.text:00007FF93E502357                 call    sub_7FF93F16DD40
		.text:00007FF93E50235C                 test    rax, rax
		.text:00007FF93E50235F                 jnz     short loc_7FF93E50236C
		.text:00007FF93E502361                 mov     rax, cs:qword_7FF93FA59C88
		.text:00007FF93E502368                 mov     rax, [rax+8]
		.text:00007FF93E50236C
		.text:00007FF93E50236C loc_7FF93E50236C:                       ; CODE XREF: GetInaccuracy+1CF↑j
		.text:00007FF93E50236C                 movss   xmm6, dword ptr [rax]
		.text:00007FF93E502370                 lea     rcx, unk_7FF93FA59C80
		.text:00007FF93E502377                 mov     rax, [rbx+380h]
		.text:00007FF93E50237E                 mov     edx, 0FFFFFFFFh
		.text:00007FF93E502383                 mulss   xmm6, dword ptr [rax+0DBCh]
		.text:00007FF93E50238B                 call    sub_7FF93E1339B0
		.text:00007FF93E502390                 mov     rax, [rbx+380h]
		.text:00007FF93E502397                 lea     rcx, unk_7FF93FAA27D0
		.text:00007FF93E50239E                 movaps  xmm10, xmm0
		.text:00007FF93E5023A2                 mov     edx, 0FFFFFFFFh
		.text:00007FF93E5023A7                 mulss   xmm10, dword ptr [rax+0DC0h]
		.text:00007FF93E5023B0                 call    sub_7FF93E1339B0
		.text:00007FF93E5023B5                 movaps  xmm1, xmm0
		.text:00007FF93E5023B8                 xorps   xmm0, xmm0
		.text:00007FF93E5023BB                 ucomiss xmm0, xmm1
		.text:00007FF93E5023BE                 ja      short loc_7FF93E5023CB
		.text:00007FF93E5023C0                 xorps   xmm9, xmm9
		.text:00007FF93E5023C4                 sqrtss  xmm9, xmm1
		.text:00007FF93E5023C9                 jmp     short loc_7FF93E5023D7
		.text:00007FF93E5023CB ; ---------------------------------------------------------------------------
		.text:00007FF93E5023CB
		.text:00007FF93E5023CB loc_7FF93E5023CB:                       ; CODE XREF: GetInaccuracy+22E↑j
		.text:00007FF93E5023CB                 movaps  xmm0, xmm1      ; X
		.text:00007FF93E5023CE                 call    sqrtf
		.text:00007FF93E5023D3                 movaps  xmm9, xmm0
		.text:00007FF93E5023D7
		.text:00007FF93E5023D7 loc_7FF93E5023D7:                       ; CODE XREF: GetInaccuracy+239↑j
		.text:00007FF93E5023D7                 xorps   xmm0, xmm0
		.text:00007FF93E5023DA                 ucomiss xmm0, xmm11
		.text:00007FF93E5023DE                 ja      short loc_7FF93E5023EA
		.text:00007FF93E5023E0                 xorps   xmm1, xmm1
		.text:00007FF93E5023E3                 sqrtss  xmm1, xmm11
		.text:00007FF93E5023E8                 jmp     short loc_7FF93E5023F6
		.text:00007FF93E5023EA ; ---------------------------------------------------------------------------
		.text:00007FF93E5023EA
		.text:00007FF93E5023EA loc_7FF93E5023EA:                       ; CODE XREF: GetInaccuracy+24E↑j
		.text:00007FF93E5023EA                 movaps  xmm0, xmm11     ; X
		.text:00007FF93E5023EE                 call    sqrtf
		.text:00007FF93E5023F3                 movaps  xmm1, xmm0
		.text:00007FF93E5023F6
		.text:00007FF93E5023F6 loc_7FF93E5023F6:                       ; CODE XREF: GetInaccuracy+258↑j
		.text:00007FF93E5023F6                 movaps  xmm2, xmm9
		.text:00007FF93E5023FA                 mulss   xmm2, cs:dword_7FF93F2A0098
		.text:00007FF93E502402                 ucomiss xmm2, xmm9
		.text:00007FF93E502406                 jp      short loc_7FF93E50241B
		.text:00007FF93E502408                 jnz     short loc_7FF93E50241B
		.text:00007FF93E50240A                 subss   xmm1, xmm9
		.text:00007FF93E50240F                 comiss  xmm1, xmm8
		.text:00007FF93E502413                 jb      short loc_7FF93E50243E
		.text:00007FF93E502415                 movaps  xmm10, xmm6
		.text:00007FF93E502419                 jmp     short loc_7FF93E50243E
		.text:00007FF93E50241B ; ---------------------------------------------------------------------------
		.text:00007FF93E50241B
		.text:00007FF93E50241B loc_7FF93E50241B:                       ; CODE XREF: GetInaccuracy+276↑j
		.text:00007FF93E50241B                                         ; GetInaccuracy+278↑j
		.text:00007FF93E50241B                 subss   xmm1, xmm2
		.text:00007FF93E50241F                 movaps  xmm0, xmm6
		.text:00007FF93E502422                 subss   xmm0, xmm10
		.text:00007FF93E502427                 subss   xmm9, xmm2
		.text:00007FF93E50242C                 mulss   xmm1, xmm0
		.text:00007FF93E502430                 divss   xmm1, xmm9
		.text:00007FF93E502435                 addss   xmm1, xmm10
		.text:00007FF93E50243A                 movaps  xmm10, xmm1
		.text:00007FF93E50243E
		.text:00007FF93E50243E loc_7FF93E50243E:                       ; CODE XREF: GetInaccuracy+283↑j
		.text:00007FF93E50243E                                         ; GetInaccuracy+289↑j
		.text:00007FF93E50243E                 comiss  xmm8, xmm10
		.text:00007FF93E502442                 jbe     short loc_7FF93E502449
		.text:00007FF93E502444                 xorps   xmm6, xmm6
		.text:00007FF93E502447                 jmp     short loc_7FF93E502452
		.text:00007FF93E502449 ; ---------------------------------------------------------------------------
		.text:00007FF93E502449
		.text:00007FF93E502449 loc_7FF93E502449:                       ; CODE XREF: GetInaccuracy+2B2↑j
		.text:00007FF93E502449                 addss   xmm6, xmm6
		.text:00007FF93E50244D                 minss   xmm6, xmm10
		.text:00007FF93E502452
		.text:00007FF93E502452 loc_7FF93E502452:                       ; CODE XREF: GetInaccuracy+2B7↑j
		.text:00007FF93E502452                 movaps  xmm10, [rsp+0C8h+var_68]
		.text:00007FF93E502458                 test    rbp, rbp
		.text:00007FF93E50245B                 jz      short loc_7FF93E502462
		.text:00007FF93E50245D
		.text:00007FF93E50245D loc_7FF93E50245D:                       ; DATA XREF: .pdata:00007FF93FC3D480↓o
		.text:00007FF93E50245D                                         ; .pdata:00007FF93FC3D48C↓o
		.text:00007FF93E50245D                 movss   dword ptr [rbp+0], xmm6
		.text:00007FF93E502462
		.text:00007FF93E502462 loc_7FF93E502462:                       ; CODE XREF: GetInaccuracy+2CB↑j
		.text:00007FF93E502462                 addss   xmm7, xmm6
		.text:00007FF93E502466
		.text:00007FF93E502466 loc_7FF93E502466:                       ; CODE XREF: GetInaccuracy+1AF↑j
		.text:00007FF93E502466                 mov     rax, [rbx+380h]
		.text:00007FF93E50246D                 lea     rcx, unk_7FF93FB0DDB8
		.text:00007FF93E502474                 mov     edx, 0FFFFFFFFh
		.text:00007FF93E502479                 mov     edi, [rax+0D40h]
		.text:00007FF93E50247F                 call    sub_7FF93F16DD40
		.text:00007FF93E502484                 movaps  xmm11, [rsp+0C8h+var_78]
		.text:00007FF93E50248A                 movaps  xmm9, [rsp+0C8h+var_58]
		.text:00007FF93E502490                 movaps  xmm6, [rsp+0C8h+var_28]
		.text:00007FF93E502498                 test    rax, rax
		.text:00007FF93E50249B                 jnz     short loc_7FF93E5024A8
		.text:00007FF93E50249D
		.text:00007FF93E50249D loc_7FF93E50249D:                       ; DATA XREF: .pdata:00007FF93FC3D48C↓o
		.text:00007FF93E50249D                                         ; .pdata:00007FF93FC3D498↓o
		.text:00007FF93E50249D                 mov     rax, cs:qword_7FF93FB0DDC0
		.text:00007FF93E5024A4                 mov     rax, [rax+8]
		.text:00007FF93E5024A8
		.text:00007FF93E5024A8 loc_7FF93E5024A8:                       ; CODE XREF: GetInaccuracy+30B↑j
		.text:00007FF93E5024A8                 cmp     byte ptr [rax], 0
		.text:00007FF93E5024AB                 jz      short loc_7FF93E5024B7
		.text:00007FF93E5024AD                 cmp     edi, 1
		.text:00007FF93E5024B0                 jle     short loc_7FF93E5024B7
		.text:00007FF93E5024B2                 addss   xmm7, xmm8
		.text:00007FF93E5024B7
		.text:00007FF93E5024B7 loc_7FF93E5024B7:                       ; CODE XREF: GetInaccuracy+31B↑j
		.text:00007FF93E5024B7                                         ; GetInaccuracy+320↑j
		.text:00007FF93E5024B7                 addss   xmm7, dword ptr [rbx+17CCh]
		.text:00007FF93E5024BF                 mov     rcx, rbx
		.text:00007FF93E5024C2                 call    sub_7FF93E503C90
		.text:00007FF93E5024C7                 addss   xmm0, xmm7
		.text:00007FF93E5024CB                 movaps  xmm7, [rsp+0C8h+var_38]
		.text:00007FF93E5024D3                 minss   xmm12, xmm0
		.text:00007FF93E5024D8                 movaps  xmm0, xmm12
		.text:00007FF93E5024DC                 movaps  xmm12, [rsp+0C8h+var_88]
		.text:00007FF93E5024E2                 jmp     short loc_7FF93E5024E7
		.text:00007FF93E5024E4 ; ---------------------------------------------------------------------------
		.text:00007FF93E5024E4
		.text:00007FF93E5024E4 loc_7FF93E5024E4:                       ; CODE XREF: GetInaccuracy+30↑j
		.text:00007FF93E5024E4                                         ; GetInaccuracy+A5↑j
		.text:00007FF93E5024E4                                         ; DATA XREF: ...
		.text:00007FF93E5024E4                 xorps   xmm0, xmm0
		.text:00007FF93E5024E7
		.text:00007FF93E5024E7 loc_7FF93E5024E7:                       ; CODE XREF: GetInaccuracy+81↑j
		.text:00007FF93E5024E7                                         ; GetInaccuracy+352↑j
		.text:00007FF93E5024E7                 lea     r11, [rsp+0C8h+var_18]
		.text:00007FF93E5024EF                 mov     rbx, [r11+28h]
		.text:00007FF93E5024F3                 movaps  xmm8, xmmword ptr [r11-30h]
		.text:00007FF93E5024F8                 mov     rsp, r11
		.text:00007FF93E5024FB                 pop     rdi
		.text:00007FF93E5024FC                 pop     rsi
		.text:00007FF93E5024FD                 pop     rbp
		.text:00007FF93E5024FE                 retn
		.text:00007FF93E5024FE GetInaccuracy   endp
		.text:00007FF93E5024FE
		.text:00007FF93E5024FE ; ---------------------------------------------------------------------------
		*/
		float x = 0.0f, y = 0.0f;
		return MEM::CallVFunc<float, 411>(this, &x, &y);
	}

	CCSWeaponBaseVData* GetWeaponVData()
	{
		return static_cast<CCSWeaponBaseVData*>(GetVData());
	}
};

class C_CSWeaponBaseGun : public C_CSWeaponBase
{
public:
	CS_CLASS_NO_INITIALIZER(C_CSWeaponBaseGun);

	SCHEMA_ADD_FIELD(std::int32_t, GetZoomLevel, "C_CSWeaponBaseGun->m_zoomLevel");
	SCHEMA_ADD_FIELD(std::int32_t, GetBurstShotsRemaining, "C_CSWeaponBaseGun->m_iBurstShotsRemaining");

	[[nodiscard]] bool CanPrimaryAttack(const int nWeaponType, const float flServerTime);
	[[nodiscard]] bool CanSecondaryAttack(const int nWeaponType, const float flServerTime);
};

class C_BaseCSGrenade : public C_CSWeaponBase
{
public:
	SCHEMA_ADD_FIELD(bool, IsHeldByPlayer, "C_BaseCSGrenade->m_bIsHeldByPlayer");
	SCHEMA_ADD_FIELD(bool, IsPinPulled, "C_BaseCSGrenade->m_bPinPulled");
	SCHEMA_ADD_FIELD(GameTime_t, GetThrowTime, "C_BaseCSGrenade->m_fThrowTime");
	SCHEMA_ADD_FIELD(float, GetThrowStrength, "C_BaseCSGrenade->m_flThrowStrength");
};

class C_BaseGrenade : public C_BaseFlex
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseGrenade);

	SCHEMA_ADD_FIELD(bool, IsSmokeGrenade, "C_BaseGrenade->m_bIsSmokeGrenade");
	SCHEMA_ADD_FIELD(bool, IsLive, "C_BaseGrenade->m_bIsLive");
	SCHEMA_ADD_FIELD(float, GetDamageRadius, "C_BaseGrenade->m_DmgRadius");
	SCHEMA_ADD_FIELD(GameTime_t, GetDetonateTime, "C_BaseGrenade->m_flDetonateTime");
	SCHEMA_ADD_FIELD(float, GetDamage, "C_BaseGrenade->m_flDamage");
	SCHEMA_ADD_FIELD(CBaseHandle, GetThrower, "C_BaseGrenade->m_hThrower");
	SCHEMA_ADD_FIELD(CBaseHandle, GetOriginalThrower, "C_BaseGrenade->m_hOriginalThrower");
};

class C_BaseCSGrenadeProjectile : public C_BaseGrenade
{
public:
	CS_CLASS_NO_INITIALIZER(C_BaseCSGrenadeProjectile);

	SCHEMA_ADD_FIELD(Vector_t, GetInitialPosition, "C_BaseCSGrenadeProjectile->m_vInitialPosition");
	SCHEMA_ADD_FIELD(Vector_t, GetInitialVelocity, "C_BaseCSGrenadeProjectile->m_vInitialVelocity");
	SCHEMA_ADD_FIELD(CUtlVectorCS2<Vector_t>, GetTrajectoryTrailPoints, "C_BaseCSGrenadeProjectile->m_arrTrajectoryTrailPoints");
};

class C_SmokeGrenadeProjectile : public C_BaseCSGrenadeProjectile
{
public:
	CS_CLASS_NO_INITIALIZER(C_SmokeGrenadeProjectile);

	SCHEMA_ADD_FIELD(bool, DidSmokeEffect, "C_SmokeGrenadeProjectile->m_bDidSmokeEffect");
	SCHEMA_ADD_FIELD(Vector_t, GetSmokeColor, "C_SmokeGrenadeProjectile->m_vSmokeColor");
	SCHEMA_ADD_FIELD(Vector_t, GetSmokeDetonationPos, "C_SmokeGrenadeProjectile->m_vSmokeDetonationPos");
};

class CModel
{
public:
	unsigned int GetBoneFlags(unsigned int idx)
	{
		using fGetBoneFlags = unsigned int(CS_FASTCALL*)(CModel*, unsigned int);
		static auto oGetBoneFlags = reinterpret_cast<fGetBoneFlags>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 41 85 C5")), 0x1));
		return oGetBoneFlags(this, idx);
	}

	const char* GetBoneName(unsigned int idx)
	{
		using fGetBoneName = const char*(CS_FASTCALL*)(CModel*, unsigned int);
		static auto oGetBoneName = reinterpret_cast<fGetBoneName>(MEM::FindPattern(CLIENT_DLL, CS_XOR("85 D2 78 25 3B 91")));
		return oGetBoneName(this, idx);
	}

	int GetBoneParent(unsigned int idx)
	{
		using fGetBoneParent = int(CS_FASTCALL*)(CModel*, unsigned int);
		static auto oGetBoneParent = reinterpret_cast<fGetBoneParent>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 41 0F 10 14 3F")), 0x1));
		return oGetBoneParent(this, idx);
	}
};

class CModelState
{
public:
	[[nodiscard]] __forceinline std::add_lvalue_reference_t<CStrongHandle<CModel>> GetModel()
	{
		static const std::uint32_t uOffset = SCHEMA::GetOffset(FNV1A::HashConst("CModelState->m_hModel")) + 0U;
		std::uint8_t* pValue = reinterpret_cast<std::uint8_t*>(this) + (uOffset);
		if (IsBadReadPtr(pValue, sizeof(CStrongHandle<CModel>)))
		{
			CStrongHandle<CModel> default_value{};
			return default_value;
		}
		return *reinterpret_cast<std::add_pointer_t<CStrongHandle<CModel>>>(pValue);
	};
};

class CSkeletonInstance : public CGameSceneNode
{
public:
	MEM_PAD(0x1CC); //0x0000
	int nBoneCount; //0x01CC
	MEM_PAD(0x18); //0x01D0
	int nMask; //0x01E8
	MEM_PAD(0x4); //0x01EC
	Matrix2x4_t* pBoneCache; //0x01F0

	SCHEMA_ADD_FIELD(CModelState, GetModelState, "CSkeletonInstance->m_modelState");

	void CalculateWorldSpaceBones(EBoneFlags mask)
	{
		using fCalculateWorldSpaceBones = void(CS_FASTCALL*)(CSkeletonInstance*, unsigned int);
		static auto oCalculateWorldSpaceBones = reinterpret_cast<fCalculateWorldSpaceBones>((MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("E8 ? ? ? ? 4C 8B AF")), 0x1)));
		oCalculateWorldSpaceBones(this, mask);
	}
};
