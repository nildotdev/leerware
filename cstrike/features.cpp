#include "features.h"

// used: draw callbacks
#include "utilities/draw.h"
// used: notify
#include "utilities/notify.h"

// used: cheat variables
#include "core/variables.h"
// used: menu
#include "core/menu.h"

// used: features callbacks
#include "features/visuals.h"
#include "features/misc.h"
#include "features/legitbot.h"

// used: interfaces
#include "core/interfaces.h"
#include "sdk/interfaces/iengineclient.h"
#include "sdk/interfaces/ccsgoinput.h"
#include "sdk/interfaces/cgameentitysystem.h"
#include "sdk/datatypes/usercmd.h"
#include "sdk/interfaces/ienginecvar.h"
#include "sdk/entity.h"
#include "features/ragebot.h"
#include "features/misc/movement.h"
#include "core/convars.h"
#include "core/sdk.h"
#include "features/rage/antiaim.h"

bool F::Setup()
{
	if (!VISUALS::Setup())
	{
		L_PRINT(LOG_ERROR) << CS_XOR("failed to setup visuals");
		return false;
	}

	return true;
}

void F::Destroy()
{
	VISUALS::OnDestroy();
}

void F::OnPresent()
{
	if (!D::bInitialized)
		return;

	D::NewFrame();
	{
		// render watermark
		MENU::RenderWatermark();

		// main window
		ImGui::PushFont(FONT::pMenu[C_GET(int, Vars.nDpiScale)]);
		// @note: here you can draw your stuff
		MENU::RenderMainWindow();
		// render notifications
		NOTIFY::Render();
		ImGui::PopFont();
	}
	D::Render();
}

void F::OnFrameStageNotify(int nStage)
{
	F::VISUALS::OnFrame(nStage);
}

// movement correction angles
static QAngle_t angCorrectionView = {};

void AutomaticWeapons(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd)
{
	C_CSWeaponBaseGun* current_weapon = SDK::LocalPawn->GetCurrentWeapon();
	if (current_weapon == nullptr)
		return;

	CCSWeaponBaseVData* vdata = current_weapon->GetWeaponVData();
	if (vdata == nullptr)
		return;

	if (vdata->IsFullAuto())
		return;

	int wepType = vdata->GetWeaponType();
	if (wepType == WEAPONTYPE_GRENADE || wepType == WEAPONTYPE_C4 || wepType == WEAPONTYPE_STACKABLEITEM)
		return;

	if (!SDK::LocalPawn->CanAttack())
	{
		pCmd->nButtons.nValue &= ~(IN_ATTACK | IN_SECOND_ATTACK);
		pCmd->nButtons.nValueChanged &= ~(IN_ATTACK | IN_SECOND_ATTACK);
		pCmd->nButtons.nValueScroll &= ~(IN_ATTACK | IN_SECOND_ATTACK);
	}
	else
	{
		pCmd->csgoUserCmd.nAttack1StartHistoryIndex = 0;
		pCmd->csgoUserCmd.nAttack3StartHistoryIndex = 0;
	}
}

void F::OnCreateMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController)
{
	F::MISC::OnMove(pCmd, pBaseCmd, pLocalController, SDK::LocalPawn);
	F::LEGITBOT::OnMove(pCmd, pBaseCmd, pLocalController, SDK::LocalPawn);
	F::RAGEBOT::OnMove(pCmd, pBaseCmd, pLocalController, SDK::LocalPawn);

	AutomaticWeapons(pCmd, pBaseCmd);

	if (const int32_t nMoveType = SDK::LocalPawn->GetMoveType(); nMoveType != MOVETYPE_NOCLIP && nMoveType != MOVETYPE_LADDER && SDK::LocalPawn->GetWaterLevel() < WL_WAIST)
		F::RAGEBOT::ANTIAIM::CorrectMovement(pCmd, pBaseCmd, I::Input->vecViewAngle);
	if (CONVAR::sv_quantize_movement_input->value.i1)
		F::MISC::MOVEMENT::AdjustAnglesForQuantizedMovement(pCmd, pBaseCmd, SDK::LocalPawn, !C_GET(bool, Vars.bAntiUntrusted));

	// loop through all tick commands
	for (int nSubTick = 0; nSubTick < pCmd->csgoUserCmd.inputHistoryField.pRep->nAllocatedSize; nSubTick++)
	{
		CCSGOInputHistoryEntryPB* pInputEntry = pCmd->GetInputHistoryEntry(nSubTick);
		if (pInputEntry == nullptr)
			continue;

		// save view angles for movement correction
		angCorrectionView = pInputEntry->pViewAngles->angValue;

		// movement correction & anti-untrusted
		F::MISC::MOVEMENT::ValidateUserCommand(pCmd, pBaseCmd, pInputEntry, angCorrectionView);
	}
}

bool F::OnDrawObject(void* pAnimatableSceneObjectDesc, void* pDx11, CMeshData* arrMeshDraw, int nDataCount, void* pSceneView, void* pSceneLayer, void* pUnk, void* pUnk2)
{
	return VISUALS::OnDrawObject(pAnimatableSceneObjectDesc, pDx11, arrMeshDraw, nDataCount, pSceneView, pSceneLayer, pUnk, pUnk2);
}
