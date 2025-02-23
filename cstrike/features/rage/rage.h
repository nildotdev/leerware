#pragma once

class CUserCmd;
class CBaseUserCmdPB;

class CCSPlayerController;
class C_CSPlayerPawn;
class HitScanResult;

namespace F::RAGEBOT::RAGE
{
	bool HitScan(HitScanResult* result, CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
	void OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
}
