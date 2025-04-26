#include "aim.h"

// used: sdk entity
#include "../../sdk/entity.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/ccsgoinput.h"
// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"

// used: activation button
#include "../../utilities/inputsystem.h"

// used: cheat variables
#include "../../core/variables.h"

// used: tracing rays
#include "../../sdk/interfaces/cgametracemanager.h"

// used: auto wall
#include "../penetration/penetration.h"

// used: accessing fixed angles
#include "../rage/antiaim.h"

// used: backtrack
#include "../lagcomp/lagcomp.h"
#include "../../core/sdk.h"

void F::LEGITBOT::AIM::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	// Check if the legitbot is enabled
	if (!C_GET(bool, Vars.bLegitbot))
		return;

	if (!pLocalController->IsPawnAlive())
		return;

	AimAssist(pBaseCmd, pLocalPawn, pLocalController);
}

QAngle_t GetRecoil(QAngle_t viewAngles, C_CSPlayerPawn* pLocal)
{
	static QAngle_t OldPunch;//get last tick AimPunch angles
	if (pLocal->GetShotsFired() >= 1)//only update aimpunch while shooting
	{
		QAngle_t delta = viewAngles - (viewAngles + (OldPunch - (pLocal->GetAimPunchAngle() * 2.f)));//get current AimPunch angles delta
		return pLocal->GetAimPunchAngle() * 2.0f;//return correct aimpunch delta
	}
	else
	{
		return QAngle_t{ 0, 0 ,0};//return 0 if is not shooting
	}
}

QAngle_t GetAngularDifference(QAngle_t vCurAngle, Vector_t vecTarget, C_CSPlayerPawn* pLocal)
{
	// The current position
	Vector_t vecCurrent = pLocal->GetEyePosition();

	// The new angle
	QAngle_t vNewAngle = (vecTarget - vecCurrent).ToAngles();
	vNewAngle.Normalize(); // Normalise it so we don't jitter about

	// Find the difference between the two angles (later useful when adding smoothing)
	vNewAngle -= vCurAngle;

	return vNewAngle;
}

void F::LEGITBOT::AIM::AimAssist(CBaseUserCmdPB* pUserCmd, C_CSPlayerPawn* pLocalPawn, CCSPlayerController* pLocalController)
{
	// Check if the activation key is down
	if (!IPT::IsKeyDown(C_GET(unsigned int, Vars.nLegitbotActivationKey)) && !C_GET(bool, Vars.bLegitbotAlwaysOn))
		return;

	// Get local player's weapon services
	CCSPlayer_WeaponServices* wep_services = pLocalPawn->GetWeaponServices();
	if (!wep_services)
		return;

	// Get the handle for the currently active weapon
	CBaseHandle wep_handle = wep_services->GetActiveWeapon();
	if (!wep_handle.IsValid())
		return;

	// Get the weapon entity of the active weapon
	auto wep = I::GameResourceService->pGameEntitySystem->Get<C_CSWeaponBase>(wep_handle);
	if (!wep)
		return;

	// Get the VData for autowall
	CCSWeaponBaseVData* vData = wep->GetWeaponVData();
	if (!vData)
		return;

	// Extract our view angles properly
	QAngle_t* pViewAngles = &I::Input->vecViewAngle;

	Vector_t eyePosition = pLocalPawn->GetEyePosition();

	// The current best distance
	float flDistance = INFINITY;
	// The target we have chosen
	CCSPlayerController* pTarget = nullptr;
	// Cache'd position
	Vector_t vecBestPosition = Vector_t();

	for (CCSPlayerController* pPlayer : SDK::PlayerControllers)
	{
		// Check the entity is not us
		if (pPlayer->IsLocalPlayerController())
			continue;

		// Get the player pawn
		C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
		if (pPawn == nullptr)
			continue;

		// Make sure they're alive
		if (!pPlayer->IsPawnAlive())
			continue;

		// Check if they're an enemy
		if (!pLocalPawn->IsOtherEnemy(pPawn))
			continue;

		// Check if they're dormant
		CGameSceneNode* pCGameSceneNode = pPawn->GetGameSceneNode();
		if (pCGameSceneNode == nullptr || pCGameSceneNode->IsDormant())
			continue;

		// Get the position

		// Firstly, get the skeleton
		CSkeletonInstance* pSkeleton = pCGameSceneNode->GetSkeletonInstance();
		if (pSkeleton == nullptr)
			continue;
		// Now the bones
		Matrix2x4_t* pBoneCache = pSkeleton->pBoneCache;
		if (pBoneCache == nullptr)
			continue;

		const int iBone = 6; // You may wish to change this dynamically but for now let's target the head.

		// Get the bone's position
		Vector_t vecPos = pBoneCache->GetOrigin(iBone);

		if (C_GET(bool, Vars.bLegitAutoWall))
		{
			F::PENETRATION::c_auto_wall AutoWall;
			F::PENETRATION::c_auto_wall::data_t hitData;
			AutoWall.pen(hitData, eyePosition, vecPos, pLocalPawn, pPawn, vData);
			if (!hitData.m_can_hit)
				continue;
		}
		else
		{
			// initialize trace, construct filter and initialize ray
			GameTrace_t trace = GameTrace_t();
			TraceFilter_t filter = TraceFilter_t(0x1C3003, pLocalPawn, nullptr, 4);
			Ray_t ray = Ray_t();

			// cast a ray from local player eye positon -> player head bone
			// @note: would recommend checking for nullptrs
			I::GameTraceManager->TraceShape(&ray, eyePosition, vecPos, &filter, &trace);
			// check if the hit entity is the one we wanted to check and if the trace end point is visible
			if (trace.m_pHitEntity != pPawn || !trace.IsVisible()) // if invisible, skip this entity
				continue;
		}

		// Get the distance/weight of the move
		float flCurrentDistance = MATH::CalculateFOVDistance(*pViewAngles, MATH::CalculateAngles(eyePosition, vecPos));
		if (flCurrentDistance > C_GET(float, Vars.flAimRange))// Skip if this move out of aim range
			continue;
		if (pTarget && flCurrentDistance > flDistance) // Override if this is the first move or if it is a better move
			continue;

		// Better move found, override.
		pTarget = pPlayer;
		flDistance = flCurrentDistance;
		vecBestPosition = vecPos;
	}

	// Check if a target was found
	if (pTarget == nullptr)
		return;

	// Point at them

	// Find the change in angles
	QAngle_t vNewAngles = GetAngularDifference(*pViewAngles, vecBestPosition, pLocalPawn);

	// Get the smoothing
	const float flSmoothing = C_GET(float, Vars.flSmoothing);
	auto aimPunch =  GetRecoil(*pViewAngles, pLocalPawn); //get AimPunch angles
	// Apply smoothing and set angles
	pViewAngles->x +=  ( vNewAngles.x - aimPunch.x ) / flSmoothing;// minus AimPunch angle to counteract recoil
	pViewAngles->y +=  ( vNewAngles.y - aimPunch.y ) / flSmoothing;
	pViewAngles->Normalize();
}
