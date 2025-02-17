#include "rage.h"

// used: cheat variables
#include "../../core/variables.h"

// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"

// used: game's sdk
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/ienginecvar.h"
#include "../../sdk/entity.h"

// used: convars
#include "../../core/convars.h"

void F::RAGEBOT::RAGE::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bRageEnable))
		return;


}
