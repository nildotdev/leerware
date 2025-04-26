#pragma once

class CUserCmd;
class CBaseUserCmdPB;

class CCSPlayerController;
class C_CSPlayerPawn;
class C_CSWeaponBase;
class HitScanResult;
class Vector_t;

namespace F::RAGEBOT::RAGE
{
	C_CSPlayerPawn* GetTarget();
	bool HitChance(C_CSPlayerPawn* pLocal, C_CSPlayerPawn* pTarget, Vector_t vTargetOrigin, float fHitchance, int nHitboxId, float* outChance);
	void AutoStop(C_CSPlayerPawn* pLocal, C_CSWeaponBase* pWeapon, CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd);
	bool HitScan(HitScanResult* result, CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
	void AutoRevolver(C_CSPlayerPawn* pLocalPawn, CUserCmd* pCmd);
	void OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
}
