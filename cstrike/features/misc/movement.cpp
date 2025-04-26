#include "movement.h"

#include "../../sdk/interfaces/ccsgoinput.h"
// used: sdk entity
#include "../../sdk/entity.h"
// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"

// used: convars
#include "../../core/convars.h"
#include "../../sdk/interfaces/ienginecvar.h"

// used: cheat variables
#include "../../core/variables.h"
#include "../rage/antiaim.h"
#include "../prediction/engine.h"

void F::MISC::MOVEMENT::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!pLocalController->IsPawnAlive())
		return;

	// check if player is in noclip or on ladder or in water
	if (const int32_t nMoveType = pLocalPawn->GetMoveType(); nMoveType == MOVETYPE_NOCLIP || nMoveType == MOVETYPE_LADDER || pLocalPawn->GetWaterLevel() >= WL_WAIST)
		return;

	AutoStrafe(pCmd, pBaseCmd, pLocalPawn);
	HalfDuck(pCmd, pLocalPawn);
	JumpBug(pCmd, pLocalPawn);
	SlowWalk(pCmd, pBaseCmd, pLocalPawn);
	QuickStop(pCmd, pBaseCmd, pLocalPawn, false);
}

void F::MISC::MOVEMENT::PostPrediction(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (pCmd == nullptr || pBaseCmd == nullptr || pLocalController == nullptr || pLocalPawn == nullptr)
		return;

	if (!pLocalController->IsPawnAlive())
		return;

	if (const int32_t nMoveType = pLocalPawn->GetMoveType(); nMoveType == MOVETYPE_NOCLIP || nMoveType == MOVETYPE_LADDER || pLocalPawn->GetWaterLevel() >= WL_WAIST)
		return;

	BunnyHop(pCmd, pLocalPawn);
	EdgeJump(pCmd, pLocalPawn);
}

void F::MISC::MOVEMENT::HalfDuck(CUserCmd* pCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bHalfDuck))
		return;

	if (pLocalPawn->GetFlags() & FL_DUCKING)
		pCmd->nButtons.nValue &= ~IN_DUCK;
}

void F::MISC::MOVEMENT::BunnyHop(CUserCmd* pCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bAutoBHop) || CONVAR::sv_autobunnyhopping->value.i1)
		return;

	if (pCmd->nButtons.nValue & IN_JUMP && pLocalPawn->GetFlags() & FL_ONGROUND)
		pCmd->nButtons.nValue &= ~IN_JUMP;
}

void F::MISC::MOVEMENT::JumpBug(CUserCmd* pCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bJumpBug))
		return;

	bool willHitGround = !(g_PredictionSystem->GetOldFlags() & FL_ONGROUND) && pLocalPawn->GetFlags() & FL_ONGROUND;
	if (willHitGround)
		pCmd->nButtons.nValue |= IN_DUCK;
}

void F::MISC::MOVEMENT::EdgeJump(CUserCmd* pCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bEdgeJump))
		return;

	if (g_PredictionSystem->GetOldFlags() & FL_ONGROUND && !(pLocalPawn->GetFlags() & FL_ONGROUND))
		pCmd->nButtons.nValue |= IN_JUMP;
}

void F::MISC::MOVEMENT::AutoStrafe(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn)
{
	static uint64_t last_pressed = 0;
	static uint64_t last_buttons = 0;

	if (!C_GET(bool, Vars.bAutoStrafe) || pLocalPawn->GetFlags() & FL_ONGROUND)
		return;

	CUserCmd* cmd = pCmd;

	const auto current_buttons = cmd->nButtons.nValue;
	auto yaw = MATH::NormalizeYaw(pUserCmd->pViewAngles->angValue.y);

	const auto check_button = [&](const uint64_t button)
	{
		if (current_buttons & button && (!(last_buttons & button) || button & IN_MOVELEFT && !(last_pressed & IN_MOVERIGHT) || button & IN_MOVERIGHT && !(last_pressed & IN_MOVELEFT) || button & IN_FORWARD && !(last_pressed & IN_BACK) || button & IN_BACK && !(last_pressed & IN_FORWARD)))
		{
			if (true)
			{
				if (button & IN_MOVELEFT)
					last_pressed &= ~IN_MOVERIGHT;
				else if (button & IN_MOVERIGHT)
					last_pressed &= ~IN_MOVELEFT;
				else if (button & IN_FORWARD)
					last_pressed &= ~IN_BACK;
				else if (button & IN_BACK)
					last_pressed &= ~IN_FORWARD;
			}

			last_pressed |= button;
		}
		else if (!(current_buttons & button))
			last_pressed &= ~button;
	};

	check_button(IN_MOVELEFT);
	check_button(IN_MOVERIGHT);
	check_button(IN_FORWARD);
	check_button(IN_BACK);

	last_buttons = current_buttons;

	const auto velocity = pLocalPawn->GetAbsVelocity();

	if (true)
	{
		auto offset = 0.f;
		if (current_buttons & IN_MOVELEFT)
			offset += 90.f;
		if (current_buttons & IN_MOVERIGHT)
			offset -= 90.f;
		if (current_buttons & IN_FORWARD)
			offset *= 0.5f;
		else if (current_buttons & IN_BACK)
			offset = -offset * 0.5f + 180.f;

		yaw += offset;

		pUserCmd->flForwardMove = 0.f;
		pUserCmd->flSideMove = 0.f;
	}

	if (pUserCmd->flSideMove != 0.0f || pUserCmd->flForwardMove != 0.0f)
		return;

	auto velocity_angle = M_RAD2DEG(std::atan2f(velocity.y, velocity.x));
	if (velocity_angle < 0.0f)
		velocity_angle += 360.0f;

	if (velocity_angle < 0.0f)
		velocity_angle += 360.0f;

	velocity_angle -= floorf(velocity_angle / 360.0f + 0.5f) * 360.0f;

	const auto speed = velocity.Length2D();
	const auto ideal = MATH::Clamp(M_RAD2DEG(std::atan2(15.f, speed)), 0.f, 45.f);

	const auto correct = (100.f - 90.f) * 0.02f * (ideal + ideal);

	pUserCmd->flForwardMove = 0.f;
	const auto velocity_delta = MATH::NormalizeYaw(yaw - velocity_angle);

	auto rotate_movement = [](CUserCmd* cmd, float target_yaw)
	{
		CBaseUserCmdPB* pBaseCmd = cmd->csgoUserCmd.pBaseCmd;
		const float rot = M_DEG2RAD(pBaseCmd->pViewAngles->angValue.y - target_yaw);

		const float new_forward = std::cos(rot) * pBaseCmd->flForwardMove - std::sin(rot) * pBaseCmd->flSideMove;
		const float new_side = std::sin(rot) * pBaseCmd->flForwardMove + std::cos(rot) * pBaseCmd->flSideMove;

		cmd->nButtons.nValue &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);
		cmd->nButtons.nValueChanged &= ~(IN_BACK | IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT);

		pBaseCmd->flForwardMove = MATH::Clamp(new_forward, -1.f, 1.f);
		pBaseCmd->flSideMove = MATH::Clamp(new_side * -1.f, -1.f, 1.f);

		if (pBaseCmd->flForwardMove > 0.f)
			cmd->nButtons.nValue |= IN_FORWARD;
		else if (pBaseCmd->flForwardMove < 0.f)
			cmd->nButtons.nValue |= IN_BACK;

		if (pBaseCmd->flSideMove > 0.f)
			cmd->nButtons.nValue |= IN_MOVELEFT;
		else if (pBaseCmd->flSideMove < 0.f)
			cmd->nButtons.nValue |= IN_MOVERIGHT;
	};

	if (fabsf(velocity_delta) > 170.f && speed > 80.f || velocity_delta > correct && speed > 80.f)
	{
		yaw = correct + velocity_angle;
		pUserCmd->flSideMove = -1.f;
		rotate_movement(cmd, MATH::NormalizeYaw(yaw));
		return;
	}

	const bool side_switch = pUserCmd->nLegacyCommandNumber % 2 == 0;

	if (-correct <= velocity_delta || speed <= 80.f)
	{
		if (side_switch)
		{
			yaw = yaw - ideal;
			pUserCmd->flSideMove = -1.f;
		}
		else
		{
			yaw = ideal + yaw;
			pUserCmd->flSideMove = 1.f;
		}
		rotate_movement(cmd, MATH::NormalizeYaw(yaw));
	}
	else
	{
		yaw = velocity_angle - correct;
		pUserCmd->flSideMove = 1.f;

		rotate_movement(cmd, MATH::NormalizeYaw(yaw));
	}
}

void F::MISC::MOVEMENT::LimitSpeed(CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn, const float speed)
{
	if (!(pLocalPawn->GetFlags() & FL_ONGROUND))
		return;

	if (CONVAR::sv_quantize_movement_input->value.i1)
	{
		float current_speed = pLocalPawn->GetAbsVelocity().Length2D();

		if (current_speed > speed)
		{
			pUserCmd->flForwardMove = 0.f;
			pUserCmd->flSideMove = 0.f;
			pUserCmd->nCachedBits &= ~(EBaseCmdBits::BASE_BITS_FORWARDMOVE | EBaseCmdBits::BASE_BITS_LEFTMOVE);
		}
	}
	else
	{
		const auto sidemove{ pUserCmd->flSideMove };
		const auto forwardmove{ pUserCmd->flForwardMove };

		const auto move_speed{ static_cast<float>(
		std::sqrt(std::pow(sidemove, 2) + std::pow(forwardmove, 2))) };
		if (move_speed > speed)
		{
			const auto invalid_speed{ speed + 1.0f < pLocalPawn->GetVecVelocity().Length2D() };

			pUserCmd->flSideMove = invalid_speed ? 0.0f : (sidemove / move_speed) * speed;
			pUserCmd->flForwardMove = invalid_speed ? 0.0f : (forwardmove / move_speed) * speed;
			pUserCmd->nCachedBits |= (EBaseCmdBits::BASE_BITS_FORWARDMOVE | EBaseCmdBits::BASE_BITS_LEFTMOVE);
		}
	}
}

void F::MISC::MOVEMENT::QuickStop(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocal, bool shouldForceStop)
{
	if (!pBaseCmd->pInButtonState)
		return;

	if (!(pLocal->GetFlags() & FL_ONGROUND))
		return;

	if (!shouldForceStop && !C_GET(bool, Vars.bQuickStop))
		return;

	bool pressing_move_keys = (pCmd->nButtons.nValue & IN_FORWARD || pCmd->nButtons.nValue & IN_MOVELEFT ||
	pCmd->nButtons.nValue & IN_BACK || pCmd->nButtons.nValue & IN_MOVERIGHT);

	Vector_t localVelocity = pLocal->GetVecVelocity();
	localVelocity.z = 0.f;
	float speed = localVelocity.Length2D();

	if ((pressing_move_keys && !shouldForceStop) || speed < 1.f)
		return;

	CPlayer_MovementServices* movesys = pLocal->GetMovementServices();
	if (!movesys)
		return;

	C_CSWeaponBaseGun* Weapon = pLocal->GetCurrentWeapon();
	if (!Weapon)
		return;

	CCSWeaponBaseVData* WpnData = Weapon->GetWeaponVData();
	if (!WpnData)
		return;

	static const auto accelerate = CONVAR::sv_accelerate->value.fl;
	static const auto max_speed = CONVAR::sv_maxspeed->value.fl;

	const auto surface_friction = movesys->GetSurfaceFriction();
	const auto max_accelspeed = accelerate * max_speed * surface_friction * I::GlobalVars->flFrameTime3;

	auto get_maximum_accelerate_speed = [&]
	{
		const auto speed_ratio{ speed / (accelerate * I::GlobalVars->flFrameTime3) };
		return speed - max_accelspeed <= -1.f ? max_accelspeed / speed_ratio : max_accelspeed;
	};

	QAngle_t velocity_angle{};
	MATH::VectorAngles(localVelocity * -1.0f, velocity_angle, nullptr);
	velocity_angle.y = I::Input->vecViewAngle.y - velocity_angle.y;

	Vector_t forward{};
	MATH::AngleVectors(velocity_angle, &forward, nullptr, nullptr);

	const auto total_speed{ get_maximum_accelerate_speed() };
	const auto max_weapon_speed = pLocal->IsScoped() ? WpnData->GetMaxSpeed()[1] : WpnData->GetMaxSpeed()[0];
	pBaseCmd->flForwardMove = (forward.x * total_speed) / max_weapon_speed;
	pBaseCmd->flSideMove = (forward.y * -total_speed) / max_weapon_speed;
	pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_FORWARDMOVE);
	pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);
}

void F::MISC::MOVEMENT::SlowWalk(CUserCmd* pCmd, CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bSlowWalk))
		return;

	if (!(pCmd->nButtons.nValue & IN_SPEED))
		return;

	pCmd->nButtons.nValue &= ~IN_SPEED;
	pCmd->nButtons.nValueChanged &= ~IN_SPEED;
	pCmd->nButtons.nValueScroll &= ~IN_SPEED;
	pUserCmd->RemoveButtonEvent(IN_SPEED);

	if (!(pLocalPawn->GetFlags() & FL_ONGROUND))
		return;

	auto weapon = pLocalPawn->GetCurrentWeapon();
	if (weapon == nullptr)
		return;

	CCSWeaponBaseVData* vData = weapon->GetWeaponVData();
	if (vData == nullptr)
		return;

	float maxSpeed = 0.33000001f * (weapon->GetZoomLevel() > 0 ? vData->GetMaxSpeed()[1] : vData->GetMaxSpeed()[0]);
	if (CONVAR::sv_quantize_movement_input->value.i1)
		LimitSpeed(pUserCmd, pLocalPawn, maxSpeed);
	else
	{
		const float speed{ pLocalPawn->GetVecVelocity().Length2D() };

		if (speed <= maxSpeed)
		{
			LimitSpeed(pUserCmd, pLocalPawn, maxSpeed);
			return;
		}

		QuickStop(pCmd, pUserCmd, pLocalPawn, true);
	}
}

void F::MISC::MOVEMENT::AdjustAnglesForQuantizedMovement(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, C_CSPlayerPawn* pLocalPawn, bool bUseRoll)
{
	if (pCmd == nullptr || pBaseCmd == nullptr || pBaseCmd->pViewAngles == nullptr)
		return;

	if (!pBaseCmd->flForwardMove && !pBaseCmd->flSideMove)
		return;

	if (pCmd->nButtons.nValue & IN_ATTACK)
		return;

	if (const int32_t nMoveType = pLocalPawn->GetMoveType(); nMoveType == MOVETYPE_NOCLIP || nMoveType == MOVETYPE_LADDER || pLocalPawn->GetWaterLevel() >= WL_WAIST)
		return;

	if (SDK::LocalController->IsThrowingGrenade())
		return;

	QAngle_t targetAngle = I::Input->vecViewAngle;
	float fwdMove = pBaseCmd->flForwardMove;
	float sideMove = pBaseCmd->flSideMove;

	Vector_t fwd, right;
	MATH::AngleVectors(targetAngle, &fwd, &right);
	Vector_t pos = pLocalPawn->GetSceneOrigin();

	Vector_t dir = fwd * fwdMove + (-right) * sideMove;
	Vector_t newPos = pos + dir;

	QAngle_t adjustedAngles = MATH::CalculateAngles(pos, newPos);

	QAngle_t* viewAngles = &pBaseCmd->pViewAngles->angValue;
	float roll = -MATH::NormalizeYaw(adjustedAngles.y + (viewAngles->y - targetAngle.y) + 90.f);
	viewAngles->x = 89.f;
	viewAngles->z = roll + viewAngles->y;
	pBaseCmd->flForwardMove = 0.f;
	pBaseCmd->flSideMove = -1.f;
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
