#include "ragebot.h"
#include "rage/antiaim.h"
#include "rage/rage.h"

void F::RAGEBOT::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	ANTIAIM::OnMove(pCmd, pBaseCmd, pLocalController, pLocalPawn);
	RAGE::OnMove(pCmd, pBaseCmd, pLocalController, pLocalPawn);
}
