#pragma once

class CUserCmd;
class CBaseUserCmdPB;

class CCSPlayerController;
class C_CSPlayerPawn;
class QAngle_t;

namespace F::RAGEBOT::ANTIAIM
{
	void OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
	bool ShouldRunAntiAim(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
	QAngle_t* GetSavedAngles();
}
