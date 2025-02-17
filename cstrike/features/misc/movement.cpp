#include "movement.h"

// used: sdk entity
#include "../../sdk/entity.h"
// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"

// used: convars
#include "../../core/convars.h"
#include "../../sdk/interfaces/ienginecvar.h"

// used: cheat variables
#include "../../core/variables.h"

void F::MISC::MOVEMENT::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!pLocalController->IsPawnAlive())
		return;

	// check if player is in noclip or on ladder or in water
	if (const int32_t nMoveType = pLocalPawn->GetMoveType(); nMoveType == MOVETYPE_NOCLIP || nMoveType == MOVETYPE_LADDER || pLocalPawn->GetWaterLevel() >= WL_WAIST)
		return;

	BunnyHop(pCmd, pBaseCmd, pLocalPawn);
	AutoStrafe(pBaseCmd, pLocalPawn);
	HalfDuck(pCmd, pLocalPawn);
}

void F::MISC::MOVEMENT::HalfDuck(CUserCmd* pCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bHalfDuck))
		return;

	if (pLocalPawn->GetFlags() & FL_DUCKING)
		pCmd->nButtons.nValue &= ~IN_DUCK;
}

void F::MISC::MOVEMENT::BunnyHop(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bAutoBHop) || CONVAR::sv_autobunnyhopping->value.i1)
		return;

	/*if (pLocalPawn->GetFlags() & FL_ONGROUND)
	{
		pCmd->nButtons.SetBits(EButtonStatePBBits::BUTTON_STATE_PB_BITS_BUTTONSTATE1);
		pCmd->nButtons.nValue &= ~IN_JUMP;
		pCmd->nButtons.nValue &= ~IN_JUMP;
	}*/

	if (pLocalPawn->GetMoveType() != MOVETYPE_WALK)
		return;

	bool jumping = pCmd->nButtons.nValue & IN_JUMP || pCmd->nButtons.nValueScroll & IN_JUMP;

	if (!jumping)
		return;

	pCmd->nButtons.nValue &= ~IN_JUMP;
	pCmd->nButtons.nValueChanged &= ~IN_JUMP;
	pCmd->nButtons.nValueScroll &= ~IN_JUMP;

	if (!(pLocalPawn->GetFlags() & FL_ONGROUND))
		return;

	pCmd->nButtons.nValue |= IN_JUMP;
	pCmd->nButtons.nValueChanged |= IN_JUMP;

	pUserCmd->WriteButtonEvent(IN_JUMP, true, 0.999f);
	pUserCmd->WriteButtonEvent(IN_JUMP, false, 0.999f);
}

void F::MISC::MOVEMENT::AutoStrafe(CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bAutoStrafe) || pLocalPawn->GetFlags() & FL_ONGROUND)
		return;

	pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
	pUserCmd->flSideMove = pUserCmd->nMousedX > 0 ? -1.0f : 1.0f; // a bit yanky, but works
}

void F::MISC::MOVEMENT::ValidateUserCommand(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputEntry, QAngle_t angCorrectionView)
{
	if (pUserCmd == nullptr)
		return;

	// clamp angle to avoid untrusted angle
	if (C_GET(bool, Vars.bAntiUntrusted))
	{
		pInputEntry->SetBits(EInputHistoryBits::INPUT_HISTORY_BITS_VIEWANGLES);
		if (pInputEntry->pViewAngles->angValue.IsValid())
		{
			pInputEntry->pViewAngles->angValue.Clamp();
			pInputEntry->pViewAngles->angValue.z = 0.f;
		}
		else
		{
			pInputEntry->pViewAngles->angValue = {};
			L_PRINT(LOG_WARNING) << CS_XOR("view angles have a NaN component, the value is reset");
		}
	}

	MovementCorrection(pUserCmd, pInputEntry, angCorrectionView);

	// correct movement buttons while player move have different to buttons values
	// clear all of the move buttons states
	pCmd->nButtons.nValue &= (~IN_FORWARD | ~IN_BACK | ~IN_LEFT | ~IN_RIGHT);

	// re-store buttons by active forward/side moves
	if (pUserCmd->flForwardMove > 0.0f)
		pCmd->nButtons.nValue |= IN_FORWARD;
	else if (pUserCmd->flForwardMove < 0.0f)
		pCmd->nButtons.nValue |= IN_BACK;

	if (pUserCmd->flSideMove > 0.0f)
		pCmd->nButtons.nValue |= IN_RIGHT;
	else if (pUserCmd->flSideMove < 0.0f)
		pCmd->nButtons.nValue |= IN_LEFT;

	if (!pInputEntry->pViewAngles->angValue.IsZero())
	{
		const float flDeltaX = std::remainderf(pInputEntry->pViewAngles->angValue.x - angCorrectionView.x, 360.f);
		const float flDeltaY = std::remainderf(pInputEntry->pViewAngles->angValue.y - angCorrectionView.y, 360.f);

		float flPitch = CONVAR::m_pitch->value.fl;
		float flYaw = CONVAR::m_yaw->value.fl;

		float flSensitivity = CONVAR::sensitivity->value.fl;
		if (flSensitivity == 0.0f)
			flSensitivity = 1.0f;

		pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_MOUSEDX);
		pUserCmd->nMousedX = static_cast<short>(flDeltaX / (flSensitivity * flPitch));

		pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_MOUSEDY);
		pUserCmd->nMousedY = static_cast<short>(-flDeltaY / (flSensitivity * flYaw));
	}
}

void F::MISC::MOVEMENT::MovementCorrection(CBaseUserCmdPB* pUserCmd, CCSGOInputHistoryEntryPB* pInputEntry, const QAngle_t& angDesiredViewPoint)
{
	if (pUserCmd == nullptr)
		return;

	Vector_t vecForward = {}, vecRight = {}, vecUp = {};
	angDesiredViewPoint.ToDirections(&vecForward, &vecRight, &vecUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecForward.z = vecRight.z = vecUp.x = vecUp.y = 0.0f;

	vecForward.NormalizeInPlace();
	vecRight.NormalizeInPlace();
	vecUp.NormalizeInPlace();

	Vector_t vecOldForward = {}, vecOldRight = {}, vecOldUp = {};
	pInputEntry->pViewAngles->angValue.ToDirections(&vecOldForward, &vecOldRight, &vecOldUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecOldForward.z = vecOldRight.z = vecOldUp.x = vecOldUp.y = 0.0f;

	vecOldForward.NormalizeInPlace();
	vecOldRight.NormalizeInPlace();
	vecOldUp.NormalizeInPlace();

	const float flPitchForward = vecForward.x * pUserCmd->flForwardMove;
	const float flYawForward = vecForward.y * pUserCmd->flForwardMove;
	const float flPitchSide = vecRight.x * pUserCmd->flSideMove;
	const float flYawSide = vecRight.y * pUserCmd->flSideMove;
	const float flRollUp = vecUp.z * pUserCmd->flUpMove;

	// solve corrected movement speed
	pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_FORWARDMOVE);
	pUserCmd->flForwardMove = vecOldForward.x * flPitchSide + vecOldForward.y * flYawSide + vecOldForward.x * flPitchForward + vecOldForward.y * flYawForward + vecOldForward.z * flRollUp;

	pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
	pUserCmd->flSideMove = vecOldRight.x * flPitchSide + vecOldRight.y * flYawSide + vecOldRight.x * flPitchForward + vecOldRight.y * flYawForward + vecOldRight.z * flRollUp;

	pUserCmd->SetBits(EBaseCmdBits::BASE_BITS_UPMOVE);
	pUserCmd->flUpMove = vecOldUp.x * flYawSide + vecOldUp.y * flPitchSide + vecOldUp.x * flYawForward + vecOldUp.y * flPitchForward + vecOldUp.z * flRollUp;
}
