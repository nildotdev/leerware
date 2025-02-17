#include "antiaim.h"

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

// used: handling OverrideView
#include "../../core/hooks.h"
#include "../../sdk/datatypes/viewsetup.h"
#include "../../core/sdk.h"
#include <DirectXMath.h>
#include "../../sdk/interfaces/cgametracemanager.h"

QAngle_t savedAngles{};

void F::RAGEBOT::ANTIAIM::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bAntiAimEnable))
		return;

	if (!pLocalPawn)
		return;

	if (!pBaseCmd->pViewAngles)
		return;

	int32_t ms_y = pBaseCmd->nMousedX;
	int32_t ms_x = pBaseCmd->nMousedY;
	savedAngles.x += ms_x * CONVAR::m_pitch->value.fl;
	savedAngles.y = MATH::NormalizeYaw(savedAngles.y - ms_y * CONVAR::m_yaw->value.fl);
	savedAngles.Clamp();
	I::Input->vecViewAngle = savedAngles.ToVector();

	if (const int32_t nMoveType = pLocalPawn->GetMoveType(); nMoveType == MOVETYPE_NOCLIP || nMoveType == MOVETYPE_LADDER || pLocalPawn->GetWaterLevel() >= WL_WAIST)
		return;

	if ((pCmd->nButtons.nValue & IN_ATTACK && pLocalPawn->CanAttack(pLocalController->GetTickBase())) || pCmd->nButtons.nValue & IN_USE)
	{
		pBaseCmd->pViewAngles->angValue = savedAngles;
		pCmd->SetSubTickAngle(savedAngles);
		return;
	}

	pBaseCmd->pViewAngles->angValue = QAngle_t(89.f, MATH::NormalizeYaw(savedAngles.y + 180.f));
	pBaseCmd->flForwardMove *= -1.f;
	pBaseCmd->flSideMove *= -1.f;
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

	angle.x = -asin(delta.z / delta.Length()) * (180.0f / 3.141592654f);
	angle.y = atan2(delta.y, delta.x) * (180.0f / 3.141592654f);

	return angle;
}

void CS_FASTCALL H::OverrideView(void* pClientModeCSNormal, CViewSetup* pSetup)
{
	const auto oOverrideView = hkOverrideView.GetOriginal();
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return oOverrideView(pClientModeCSNormal, pSetup);

	if (C_GET(bool, Vars.bAntiAimEnable))
		pSetup->angView = savedAngles;
	else
		savedAngles = pSetup->angView;

	I::Input->bInThirdPerson = C_GET(bool, Vars.bThirdPerson);
	if (C_GET(bool, Vars.bThirdPerson))
	{
		Vector_t eyePos = SDK::LocalPawn->GetEyePosition();
		QAngle_t adjusted_cam_view_angle = savedAngles;
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
