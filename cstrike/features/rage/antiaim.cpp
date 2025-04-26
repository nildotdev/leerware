#include "antiaim.h"

// used: cheat variables
#include "../../core/variables.h"

// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"

// used: game's sdk
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/ienginecvar.h"
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../sdk/entity.h"

// used: convars
#include "../../core/convars.h"

// used: handling OverrideView
#include "../../sdk/interfaces/cgametracemanager.h"
#include "../../sdk/datatypes/viewsetup.h"
#include "../../core/hooks.h"
#include "../../core/sdk.h"
#include <DirectXMath.h>
#include "rage.h"
#include "../../utilities/inputsystem.h"

void F::RAGEBOT::ANTIAIM::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bAntiAimEnable))
		return;

	if (pLocalPawn == nullptr)
		return;

	if (pBaseCmd->pViewAngles == nullptr)
		return;

	int32_t ms_y = pBaseCmd->nMousedX;
	int32_t ms_x = pBaseCmd->nMousedY;

	if (!ShouldRunAntiAim(pCmd, pBaseCmd, pLocalController, pLocalPawn))
		return;

	// -2182423346297399750336966557899.f for fake up, INT_MIN for fake down
	float targetYaw = pBaseCmd->pViewAngles->angValue.y + 180;
	C_CSPlayerPawn* currentTarget = F::RAGEBOT::RAGE::GetTarget();
	if (currentTarget != nullptr)
		targetYaw = MATH::CalculateAngles(pLocalPawn->GetSceneOrigin(), currentTarget->GetSceneOrigin()).y + 180;

	static char nAntiAimState = 0;
	static char lastTarget = 0;
	static char targetState = 0;

	static bool wasPressingLeft = false;
	static bool wasPressingRight = false;

	bool isPressingLeft = IPT::IsKeyDown(C_GET(unsigned int, Vars.nOverrideLeft));
	bool isPressingRight = IPT::IsKeyDown(C_GET(unsigned int, Vars.nOverrideRight));

	if (isPressingLeft && !wasPressingLeft)
	{
		if (targetState == -1)
			targetState = 0;
		else
			targetState = -1;
	}

	if (isPressingRight && !wasPressingRight)
	{
		if (targetState == 1)
			targetState = 0;
		else
			targetState = 1;
	}

	if (targetState != lastTarget)
	{
		nAntiAimState = targetState;
		lastTarget = targetState;
	}

	wasPressingLeft = isPressingLeft;
	wasPressingRight = isPressingRight;

	switch (nAntiAimState)
	{
	case -1:
		targetYaw -= 90;
		break;
	case 1:
		targetYaw += 90;
		break;
	case 2:
		targetYaw += 180;
		break;
	}

	pBaseCmd->pViewAngles->angValue = QAngle_t(89.f, MATH::NormalizeYaw(targetYaw));
}

void MATH::AngleVectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up) // now WHY do we have to redefine this?
{
	float cp = std::cos(M_DEG2RAD(angles.x)), sp = std::sin(M_DEG2RAD(angles.x));
	float cy = std::cos(M_DEG2RAD(angles.y)), sy = std::sin(M_DEG2RAD(angles.y));
	float cr = std::cos(M_DEG2RAD(angles.z)), sr = std::sin(M_DEG2RAD(angles.z));

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = -1.f * sr * sp * cy + -1.f * cr * -sy;
		right->y = -1.f * sr * sp * sy + -1.f * cr * cy;
		right->z = -1.f * sr * cp;
	}

	if (up)
	{
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}

void F::RAGEBOT::ANTIAIM::CorrectMovement(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, QAngle_t angle)
{
	if (pCmd == nullptr || pBaseCmd == nullptr || pBaseCmd->pViewAngles == nullptr)
		return;

	QAngle_t wish_angle;
	wish_angle = pBaseCmd->pViewAngles->angValue;
	wish_angle = QAngle_t(wish_angle.x, MATH::NormalizeYaw(wish_angle.y), .0f);
	int revers = wish_angle.x > 89.f ? -1 : 1;
	wish_angle.Clamp();

	Vector_t view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
	auto viewangles = angle;

	MATH::AngleVectors(wish_angle, &view_fwd, &view_right, &view_up);
	MATH::AngleVectors(viewangles, &cmd_fwd, &cmd_right, &cmd_up);

	const float v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
	const float v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
	const float v12 = sqrtf(view_up.z * view_up.z);

	const Vector_t norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
	const Vector_t norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
	const Vector_t norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);

	const float v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
	const float v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
	const float v18 = sqrtf(cmd_up.z * cmd_up.z);

	const Vector_t norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
	const Vector_t norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
	const Vector_t norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);

	const float v22 = norm_view_fwd.x * pBaseCmd->flForwardMove;
	const float v26 = norm_view_fwd.y * pBaseCmd->flForwardMove;
	const float v28 = norm_view_fwd.z * pBaseCmd->flForwardMove;
	const float v24 = norm_view_right.x * pBaseCmd->flSideMove;
	const float v23 = norm_view_right.y * pBaseCmd->flSideMove;
	const float v25 = norm_view_right.z * pBaseCmd->flSideMove;
	const float v30 = norm_view_up.x * pBaseCmd->flUpMove;
	const float v27 = norm_view_up.z * pBaseCmd->flUpMove;
	const float v29 = norm_view_up.y * pBaseCmd->flUpMove;

	pBaseCmd->flForwardMove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25)) + (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28))) + (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
	pBaseCmd->flSideMove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25)) + (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28))) + (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));

	pBaseCmd->flUpMove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25)) + (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28))) + (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));
	pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_UPMOVE);

	pBaseCmd->flForwardMove = revers * ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25)) + (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)));

	pBaseCmd->flSideMove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25)) + (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)));

	pBaseCmd->flForwardMove = MATH::Clamp(pBaseCmd->flForwardMove, -1.f, 1.f);
	pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_FORWARDMOVE);

	pBaseCmd->flSideMove = MATH::Clamp(pBaseCmd->flSideMove, -1.f, 1.f);
	pBaseCmd->SetBits(EBaseCmdBits::BASE_BITS_LEFTMOVE);

	// correct movement buttons while player move have different to buttons values
	// clear all of the move buttons states

	pCmd->nButtons.nValue &= (~IN_FORWARD | ~IN_BACK | ~IN_MOVELEFT | ~IN_MOVERIGHT);

	// re-store buttons by active forward/side moves
	if (pBaseCmd->flForwardMove > 0.0f)
		pCmd->nButtons.nValue |= IN_FORWARD;
	else if (pBaseCmd->flForwardMove < 0.0f)
		pCmd->nButtons.nValue |= IN_BACK;

	if (pBaseCmd->flSideMove > 0.0f)
		pCmd->nButtons.nValue |= IN_MOVERIGHT;
	else if (pBaseCmd->flSideMove < 0.0f)
		pCmd->nButtons.nValue |= IN_MOVELEFT;
}

bool F::RAGEBOT::ANTIAIM::ShouldRunAntiAim(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!pCmd || !pBaseCmd || !pLocalController || !pLocalPawn)
		return false;

	if (pCmd->nButtons.nValue & IN_USE)
		return false;

	if (!pLocalController->IsPawnAlive())
		return false;

	if (const int32_t nMoveType = pLocalPawn->GetMoveType(); nMoveType == MOVETYPE_NOCLIP || nMoveType == MOVETYPE_LADDER || pLocalPawn->GetWaterLevel() >= WL_WAIST)
		return false;

	CCSPlayer_WeaponServices* wep_services = pLocalPawn->GetWeaponServices();
	if (!wep_services)
		return true; // There's a reason for this.

	CBaseHandle wep_handle = wep_services->GetActiveWeapon();
	if (!wep_handle.IsValid())
		return true; // And this.

	auto wep = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(wep_handle);
	if (!wep)
		return true; // And this.

	CCSWeaponBaseVData* vData = wep->GetWeaponVData();
	if (!vData)
		return true; // There's a reason for everything. Just ask me if you're confused.

	if (vData->GetWeaponType() == WEAPONTYPE_GRENADE)
	{
		if (pLocalController->IsThrowingGrenade())
			return false;

		/*if (pLocalPawn->GetFlags() & FL_ONGROUND && pCmd->nButtons.nValue & IN_JUMP)
			return false;*/

		return true;
	}

	if (pCmd->nButtons.nValue & IN_ATTACK && pLocalPawn->CanAttack())
		return false;

	return true;
}

Vector_t CalculateCameraPosition(Vector_t anchorPos, float distance, QAngle_t viewAngles)
{
	float yaw = DirectX::XMConvertToRadians(viewAngles.y);
	float pitch = DirectX::XMConvertToRadians(viewAngles.x);

	float x = anchorPos.x + distance * cosf(yaw) * cosf(pitch);
	float y = anchorPos.y + distance * sinf(yaw) * cosf(pitch);
	float z = anchorPos.z + distance * sinf(pitch);

	return Vector_t{ x, y, z };
}

inline QAngle_t CalcAngles(Vector_t viewPos, Vector_t aimPos)
{
	QAngle_t angle = { 0, 0, 0 };

	Vector_t delta = aimPos - viewPos;

	angle.x = -asin(delta.z / delta.Length()) * (180.0f / MATH::_PI);
	angle.y = atan2(delta.y, delta.x) * (180.0f / MATH::_PI);

	return angle;
}

void CS_FASTCALL H::OverrideView(void* pClientModeCSNormal, CViewSetup* pSetup)
{
	const auto oOverrideView = hkOverrideView.GetOriginal();
	float aspectRatio = C_GET(float, Vars.flAspectRatio);
	if (aspectRatio > 0)
	{
		pSetup->nFlags |= 2;
		pSetup->flAspectRatio = aspectRatio;
	}
	else
		pSetup->nFlags &= ~2;
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame() || SDK::LocalController == nullptr || !SDK::LocalController->IsPawnAlive())
		return oOverrideView(pClientModeCSNormal, pSetup);

	bool inThirdPerson = C_GET(bool, Vars.bThirdPerson);

	I::Input->bInThirdPerson = inThirdPerson;
	if (inThirdPerson)
	{
		Vector_t eyePos = SDK::LocalPawn->GetEyePosition();
		QAngle_t adjusted_cam_view_angle = I::Input->vecViewAngle;
		adjusted_cam_view_angle.x = -adjusted_cam_view_angle.x;
		pSetup->vecOrigin = CalculateCameraPosition(eyePos, -C_GET(int, Vars.nThirdPersonDistance), adjusted_cam_view_angle);

		Ray_t ray{};
		GameTrace_t trace{};
		TraceFilter_t filter{ 0x1C3003, SDK::LocalPawn, nullptr, 4 };

		Vector_t direction = (eyePos - pSetup->vecOrigin).Normalized();

		Vector_t temp = pSetup->vecOrigin + direction * -10.f;

		if (I::GameTraceManager->TraceShape(&ray, eyePos, pSetup->vecOrigin, &filter, &trace))
		{
			if (trace.m_pHitEntity != nullptr)
			{
				pSetup->vecOrigin = trace.m_vecPosition + (direction * 10.f);
			}
		}

		QAngle_t p = CalcAngles(pSetup->vecOrigin, eyePos);
		pSetup->angView = QAngle_t{ p.x, MATH::NormalizeYaw(p.y) };
		pSetup->angView.Clamp();
		I::Input->angThirdPersonAngles = pSetup->angView;
	}

	oOverrideView(pClientModeCSNormal, pSetup);
}

void CS_FASTCALL H::ValidateInput(CCSGOInput* pInput, int a1)
{
	const auto oValidateInput = hkValidateInput.GetOriginal();
	QAngle_t original = pInput->vecViewAngle;
	oValidateInput(pInput, a1);
	pInput->vecViewAngle = original;
}
