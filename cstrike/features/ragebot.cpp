#include "ragebot.h"
#include "rage/antiaim.h"

void F::RAGEBOT::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	F::RAGEBOT::ANTIAIM::OnMove(pCmd, pBaseCmd, pLocalController, pLocalPawn);
}
