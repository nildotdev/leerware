#pragma once

class CUserCmd;
class CBaseUserCmdPB;
class CCSPlayerController;
class C_CSPlayerPawn;

namespace F::RAGEBOT
{
	void OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn);
}
