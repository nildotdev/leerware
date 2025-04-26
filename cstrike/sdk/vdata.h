#pragma once

// used: schema field
#include "../core/schema.h"
// used: rop
#include "../utilities/memory.h"

using CFiringModeFloat = std::float_t[2];
using CSkillFloat = std::float_t[4];

class CBasePlayerVData
{
public:
	CS_CLASS_NO_INITIALIZER(CBasePlayerVData);

	//CResourceNameTyped< CWeakHandle< InfoForResourceTypeCModel > > m_sModelName = 0x28
	//CSkillFloat m_flHeadDamageMultiplier = 0x108
	//CSkillFloat m_flChestDamageMultiplier = 0x118
	//CSkillFloat m_flStomachDamageMultiplier = 0x128
	//CSkillFloat m_flArmDamageMultiplier = 0x138
	//CSkillFloat m_flLegDamageMultiplier = 0x148
	//float32 m_flHoldBreathTime = 0x158
	//float32 m_flDrowningDamageInterval = 0x15C
	//int32 m_nDrowningDamageInitial = 0x160
	//int32 m_nDrowningDamageMax = 0x164
	//int32 m_nWaterSpeed = 0x168
	//float32 m_flUseRange = 0x16C
	//float32 m_flUseAngleTolerance = 0x170
	//float32 m_flCrouchTime = 0x174
};

class CBasePlayerWeaponVData
{
public:
	CS_CLASS_NO_INITIALIZER(CBasePlayerWeaponVData);

	SCHEMA_ADD_FIELD(std::int32_t, GetMaxClip1, "CBasePlayerWeaponVData->m_iMaxClip1");
};

class CCSWeaponBaseVData : public CBasePlayerWeaponVData
{
public:
	CS_CLASS_NO_INITIALIZER(CCSWeaponBaseVData);

	SCHEMA_ADD_FIELD(const char*, GetWeaponName, "CCSWeaponBaseVData->m_szName");
	SCHEMA_ADD_FIELD(std::int32_t, GetWeaponType, "CCSWeaponBaseVData->m_WeaponType");
	SCHEMA_ADD_FIELD(CFiringModeFloat, GetWeaponSpread, "CCSWeaponBaseVData->m_flSpread");
	SCHEMA_ADD_FIELD(float, GetRange, "CCSWeaponBaseVData->m_flRange");
	SCHEMA_ADD_FIELD(CFiringModeFloat, GetMaxSpeed, "CCSWeaponBaseVData->m_flMaxSpeed");
	SCHEMA_ADD_FIELD(CFiringModeFloat, GetInaccuracyStanding, "CCSWeaponBaseVData->m_flInaccuracyStand");
	SCHEMA_ADD_FIELD(CFiringModeFloat, GetInaccuracyJumpApex, "CCSWeaponBaseVData->m_flInaccuracyJumpApex");
	SCHEMA_ADD_FIELD(CFiringModeFloat, GetInaccuracyCrouching, "CCSWeaponBaseVData->m_flInaccuracyCrouch");
	SCHEMA_ADD_FIELD(float, GetRangeModifier, "CCSWeaponBaseVData->m_flRangeModifier");
	SCHEMA_ADD_FIELD(float, GetPenetration, "CCSWeaponBaseVData->m_flPenetration");
	SCHEMA_ADD_FIELD(float, GetArmorRatio, "CCSWeaponBaseVData->m_flArmorRatio");
	SCHEMA_ADD_FIELD(float, GetHeadshotMultiplier, "CCSWeaponBaseVData->m_flHeadshotMultiplier");
	SCHEMA_ADD_FIELD(int, GetDamage, "CCSWeaponBaseVData->m_nDamage");
	SCHEMA_ADD_FIELD(bool, IsFullAuto, "CCSWeaponBaseVData->m_bIsFullAuto");
	SCHEMA_ADD_FIELD(bool, IsRevolver, "CCSWeaponBaseVData->m_bIsRevolver");
};
