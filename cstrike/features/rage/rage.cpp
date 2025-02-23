#include "rage.h"

// used: cheat variables
#include "../../core/variables.h"

// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"

// used: game's sdk
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/ienginecvar.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/entity.h"

// used: convars
#include "../../core/convars.h"

class HitScanResult
{
public:
	QAngle_t aimAngle;
	C_CSPlayerPawn* target;
	int LCTick;
	bool canHit;
	short hitbox;
};

class HitScanTarget
{
public:
	CBaseHandle entity;
};

HitScanTarget scanTarget{};

void PickScanTarget(CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	scanTarget.entity = CBaseHandle{}; // Invalidate current target
	float flDistRecord = INFINITY;

	// Get the index of the last entity
	const int nEntityCount = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
	for (int nIndex = 1; nIndex <= nEntityCount; nIndex++)
	{
		// Get the entity
		C_BaseEntity* pEntity = I::GameResourceService->pGameEntitySystem->Get(nIndex);
		if (pEntity == nullptr)
			continue;

		// Get the class info
		SchemaClassInfoData_t* pClassInfo = nullptr;
		pEntity->GetSchemaClassInfo(&pClassInfo);
		if (pClassInfo == nullptr)
			continue;

		// Get the hashed name
		const FNV1A_t uHashedName = FNV1A::Hash(pClassInfo->szName);

		// Make sure they're a player controller
		if (uHashedName != FNV1A::HashConst("CCSPlayerController"))
			continue;

		// Cast to player controller
		CCSPlayerController* pPlayer = reinterpret_cast<CCSPlayerController*>(pEntity);
		if (pPlayer == nullptr)
			continue;

		// Check the entity is not us
		if (pPlayer == pLocalController)
			continue;

		// Make sure they're alive
		if (!pPlayer->IsPawnAlive())
			continue;

		// Get the player pawn
		C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
		if (pPawn == nullptr)
			continue;

		// Check if they're an enemy
		if (!pLocalPawn->IsOtherEnemy(pPawn))
			continue;

		Vector_t localPos = pLocalPawn->GetSceneOrigin();
		Vector_t enemyPos = pPawn->GetSceneOrigin();
		float distance = localPos.DistTo(enemyPos);
		if (flDistRecord > distance)
		{
			flDistRecord = distance;
			scanTarget.entity = pPawn->GetRefEHandle();
		}
	}
}

bool F::RAGEBOT::RAGE::HitScan(HitScanResult* result, CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	C_CSPlayerPawn* target = result->target;
	if (target == nullptr)
		return false;

	CGameSceneNode* node = target->GetGameSceneNode();
	if (node == nullptr)
		return false;

	CSkeletonInstance* skeleton = node->GetSkeletonInstance();
	if (skeleton == nullptr)
		return false;

	return true;
}

void F::RAGEBOT::RAGE::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bRageEnable))
		return;

	if (!scanTarget.entity.IsValid())
		PickScanTarget(pLocalController, pLocalPawn);

	C_CSPlayerPawn* target = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(scanTarget.entity);
	if (target->GetHealth() < 1)
	{
		PickScanTarget(pLocalController, pLocalPawn);
		if (!scanTarget.entity.IsValid())
			return;
		target = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(scanTarget.entity);
	}

	if (pCmd->nButtons.nValue & IN_DUCK)
	{

	}

	HitScanResult result{};
	result.target = target;
	if (!HitScan(&result, pCmd, pBaseCmd, pLocalController, pLocalPawn))
		return;

	L_PRINT(LOG_INFO) << CS_XOR("Can hit!");
}
