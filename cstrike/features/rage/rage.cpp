#include "rage.h"
#include <string>
#include <sstream>

// used: cheat variables
#include "../../core/variables.h"

// used: cusercmd
#include "../../sdk/datatypes/usercmd.h"

// used: game's sdk
#include "../../sdk/interfaces/ccsgoinput.h"
#include "../../sdk/interfaces/iengineclient.h"
#include "../../sdk/interfaces/ienginecvar.h"
#include "../../sdk/interfaces/isource2client.h"
#include "../../sdk/interfaces/cgameentitysystem.h"
#include "../../sdk/interfaces/cdebugoverlaygamesystem.h"
#include "../../sdk/entity.h"

// used: convars
#include "../../core/convars.h"

// used: auto-wall
#include "../penetration/penetration.h"
#include "../../sdk/interfaces/cgametracemanager.h"

// used: backtrack
#include "../lagcomp/lagcomp.h"

// used: auto-stop
#include "../misc/movement.h"

// used: cheat SDK
#include "../../core/sdk.h"

// used: setting binds
#include "../../utilities/inputsystem.h"

class HitScanResult
{
public:
	QAngle_t aimAngle;
	C_CSPlayerPawn* target;
	int LCTick;
	float LCSimTime;
	bool canHit;
	short hitbox;
	float hitChance;
	int damage;
};

class HitScanTarget
{
public:
	CBaseHandle entity;
};

HitScanTarget scanTarget{};

C_CSPlayerPawn* F::RAGEBOT::RAGE::GetTarget()
{
	CBaseHandle target = scanTarget.entity;
	if (!target.IsValid())
		return nullptr;

	C_CSPlayerPawn* pawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(target);
	return pawn;
}

void PickScanTarget(CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	scanTarget.entity = CBaseHandle{}; // Invalidate current target
	float flDistRecord = INFINITY;

	ImVec2 screenSize = ImGui::GetIO().DisplaySize;
	ImVec2 screenCenter = ImVec2(screenSize.x / 2, screenSize.y / 2);
	for (CCSPlayerController* pPlayer : SDK::PlayerControllers)
	{
		// Check the entity is not us
		if (pPlayer->IsLocalPlayerController())
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
		QAngle_t aimAngle = MATH::CalculateAngles(localPos, enemyPos);
		float distance = MATH::CalculateFOVDistance(I::Input->vecViewAngle, aimAngle);

		/*ImVec2 abc{};
		if (!D::WorldToScreen(enemyPos, &abc))
			continue;
		
		float distance = sqrtf(powf(screenCenter.x - abc.x, 2.f) + powf(screenCenter.y - abc.y, 2.f));*/
		if (flDistRecord > distance)
		{
			flDistRecord = distance;
			scanTarget.entity = pPawn->GetRefEHandle();
		}
	}
}

CS_INLINE Vector_t CalculateSpread(C_CSWeaponBase* weapon, int seed, float inaccuracy, float spread, bool revolver2 = false)
{
	const char* item_def_index;
	float recoil_index, r1, r2, r3, r4, s1, c1, s2, c2;

	if (!weapon)
		return {};
	// if we have no bullets, we have no spread.
	auto wep_info = weapon->GetWeaponVData();
	if (!wep_info)
		return {};

	// get some data for later.
	item_def_index = wep_info->GetWeaponName();
	recoil_index = weapon->GetRecoilIndex();

	MATH::fnRandomSeed((seed & 0xff) + 1);

	// generate needed floats.
	r1 = MATH::fnRandomFloat(0.f, 1.f);
	r2 = MATH::fnRandomFloat(0.f, MATH::_PI * 2);
	r3 = MATH::fnRandomFloat(0.f, 1.f);
	r4 = MATH::fnRandomFloat(0.f, MATH::_PI * 2);

	// revolver secondary spread.
	if (item_def_index == CS_XOR("weapon_revoler") && revolver2)
	{
		r1 = 1.f - (r1 * r1);
		r3 = 1.f - (r3 * r3);
	}

	// negev spread.
	else if (item_def_index == CS_XOR("weapon_negev") && recoil_index < 3.f)
	{
		for (int i{ 3 }; i > recoil_index; --i)
		{
			r1 *= r1;
			r3 *= r3;
		}

		r1 = 1.f - r1;
		r3 = 1.f - r3;
	}

	// get needed sine / cosine values.
	c1 = std::cos(r2);
	c2 = std::cos(r4);
	s1 = std::sin(r2);
	s2 = std::sin(r4);

	// calculate spread vector.
	return {
		(c1 * (r1 * inaccuracy)) + (c2 * (r3 * spread)),
		(s1 * (r1 * inaccuracy)) + (s2 * (r3 * spread)),
		0.f
	};
}

bool F::RAGEBOT::RAGE::HitChance(C_CSPlayerPawn* pLocal, C_CSPlayerPawn* pTarget, Vector_t vTargetOrigin, float fHitchance, int nHitboxId, float* outChance)
{
	if (pLocal == nullptr)
		return false;

	C_CSWeaponBase* weapon = pLocal->GetCurrentWeapon();
	if (weapon == nullptr)
		return false;

	auto data = weapon->GetWeaponVData();
	if (data == nullptr)
		return false;

	CBaseHandle enemyHandle = pTarget->GetRefEHandle();
	if (!enemyHandle.IsValid())
		return false;

	int enemyEntry = enemyHandle.GetEntryIndex();

	float HITCHANCE_MAX = 100.f;
	constexpr int SEED_MAX = 255;

	Vector_t eyePos = pLocal->GetEyePosition();
	Vector_t start{ eyePos }, end, fwd, right, up, dir, wep_spread;
	float inaccuracy, spread;

	if (fHitchance <= 0)
		return true;

	if (eyePos.DistTo(vTargetOrigin) > data->GetRange())
		return false;

	unsigned int total_hits{}, needed_hits{ (unsigned int)std::ceil((fHitchance * SEED_MAX) / HITCHANCE_MAX) };

	QAngle_t vAimpoint;
	MATH::VectorAngles(eyePos - vTargetOrigin, vAimpoint, nullptr);
	MATH::AngleVectors(vAimpoint, &fwd, &right, &up);

	inaccuracy = weapon->GetInaccuracy();
	spread = weapon->GetSpread();

	for (int i{}; i <= SEED_MAX; ++i)
	{
		wep_spread = CalculateSpread(weapon, i, inaccuracy, spread);

		dir = (fwd + (right * wep_spread.x) + (up * wep_spread.y)).Normalized();

		end = start - (dir * 65536.f);

		TraceFilter_t filter(0x1C3003, pLocal, nullptr, 4);
		GameTrace_t trace = {};
		Ray_t ray = {};

		ImVec2 start_w2s{};
		ImVec2 end_w2s{};

		I::GameTraceManager->ClipRayToEntity(&ray, start, end, pTarget, &filter, &trace);
		if (!trace.m_pHitEntity)
			continue;

		CBaseHandle hitEntity = trace.m_pHitEntity->GetRefEHandle();
		if (!hitEntity.IsValid())
			continue;

		int hitboxHit = trace.GetHitboxId();
		if (hitEntity.GetEntryIndex() == enemyEntry && (nHitboxId == -1 || hitboxHit == nHitboxId))
			total_hits++;

		*outChance = static_cast<float>(total_hits) / SEED_MAX;
		if (total_hits >= needed_hits)
			return true;

		if ((SEED_MAX - i + total_hits) < needed_hits)
			return false;
	}
}

unsigned short HitboxBoneToHitboxId(unsigned int boneId)
{
	switch (boneId)
	{
	case 6:
		return EHitboxes::HITBOX_HEAD;
	case 5:
		return EHitboxes::HITBOX_NECK;
	case 0:
		return EHitboxes::HITBOX_PELVIS;
	case 1:
		return EHitboxes::HITBOX_PELVIS1;
	case 2:
		return EHitboxes::HITBOX_STOMACH;
	case 3:
		return EHitboxes::HITBOX_CHEST;
	case 4:
		return EHitboxes::HITBOX_CHEST1;
	case 22:
		return EHitboxes::HITBOX_LEFTUPPERLEG;
	case 25:
		return EHitboxes::HITBOX_RIGHTUPPERLEG;
	case 23:
		return EHitboxes::HITBOX_LEFTLOWERLEG;
	case 26:
		return EHitboxes::HITBOX_RIGHTLOWERLEG;
	case 24:
		return EHitboxes::HITBOX_LEFTTOE;
	case 27:
		return EHitboxes::HITBOX_RIGHTTOE;
	case 10:
		return EHitboxes::HITBOX_LEFTHAND;
	case 15:
		return EHitboxes::HITBOX_RIGHTHAND;
	case 8:
		return EHitboxes::HITBOX_LEFTUPPERARM;
	case 9:
		return EHitboxes::HITBOX_LEFTLOWERARM;
	case 13:
		return EHitboxes::HITBOX_RIGHTUPPERARM;
	case 14:
		return EHitboxes::HITBOX_RIGHTLOWERARM;
	default:
		return -1;
	}
}

void F::RAGEBOT::RAGE::AutoStop(C_CSPlayerPawn* pLocal, C_CSWeaponBase* pWeapon, CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd)
{
	auto WeaponVData = pWeapon->GetWeaponVData();
	if (!WeaponVData)
		return;

	Vector_t localVelocity = pLocal->GetVecVelocity();
	localVelocity.z = 0.f;
	float speed = localVelocity.Length2D();
	int fireType = 0;
	if (WeaponVData->GetWeaponType() == WEAPONTYPE_SNIPER_RIFLE)
		fireType = 1;
	if (pWeapon->IsBurstMode())
		fireType = 1;
	float maxSpeed = WeaponVData->GetMaxSpeed()[fireType] * 0.33000001f;

	if (speed <= maxSpeed)
		F::MISC::MOVEMENT::LimitSpeed(pBaseCmd, pLocal, maxSpeed);
	else
		F::MISC::MOVEMENT::QuickStop(pCmd, pBaseCmd, pLocal, true);
}

float currentFraction = 0.f;
constexpr unsigned int HITBOX_MAX = 18; // Something like that?

bool F::RAGEBOT::RAGE::HitScan(HitScanResult* result, CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (pLocalPawn == nullptr)
		return false;

	C_CSWeaponBaseGun* weapon = pLocalPawn->GetCurrentWeapon();
	if (weapon == nullptr)
		return false;

	C_AttributeContainer* attribs = weapon->GetAttributeManager();
	if (attribs == nullptr)
		return false;

	C_EconItemView* item = attribs->GetItem();
	if (item == nullptr)
		return false;

	CCSWeaponBaseVData* vData = weapon->GetWeaponVData();
	if (vData == nullptr)
		return false;

	C_CSPlayerPawn* target = result->target;
	if (target == nullptr)
		return false;

	CGameSceneNode* node = target->GetGameSceneNode();
	if (node == nullptr)
		return false;

	CSkeletonInstance* skeleton = node->GetSkeletonInstance();
	if (skeleton == nullptr)
		return false;

	short currentWeapon = item->GetItemDefinitionIndex();
	size_t rageVariableIndex = Vars.varsGlobal;

	switch (currentWeapon)
	{
	case WEAPON_SSG_08:
		rageVariableIndex = Vars.varsScout;
		break;
	case WEAPON_G3SG1:
	case WEAPON_SCAR_20:
		rageVariableIndex = Vars.varsAuto;
		break;
	case WEAPON_AWP:
		rageVariableIndex = Vars.varsAWP;
		break;
	case WEAPON_DESERT_EAGLE:
		rageVariableIndex = Vars.varsDeagle;
		break;
	case WEAPON_R8_REVOLVER:
		rageVariableIndex = Vars.varsR8;
		break;
	default:
		if (vData->GetWeaponType() == WEAPONTYPE_PISTOL)
			rageVariableIndex = Vars.varsPistols;
		break;
	}

	RageBotVars_t variables = C_GET(RageBotVars_t, rageVariableIndex);
	if (!variables.bEnable)
		variables = C_GET(RageBotVars_t, Vars.varsGlobal);

	int multiPoint = variables.nMultipoint;
	if (multiPoint == -1)
		multiPoint = 80;

	int hitChance = variables.nHitChance;
	if (hitChance == -1)
		hitChance = MATH::Clamp((int)roundf(vData->GetInaccuracyStanding()[0] * 10000.f), 0, 100);

	int minDamage = IPT::GetBindState(C_GET(KeyBind_t, Vars.kMinDamageOverride)) ? variables.nMinDamageOverride : variables.nMinDamage;
	if (minDamage == 0)
		minDamage = vData->GetDamage();

	if (minDamage > 100)
		minDamage = target->GetHealth() + (minDamage - 100);

	Vector_t shootPos = pLocalPawn->GetEyePosition();
	QAngle_t aimPunch{};
	auto cache = pLocalPawn->GetAimPunchCache();
	if (cache.m_Size > 0 && cache.m_Size <= 0xFFFF)
		aimPunch = cache.m_Data[cache.m_Size - 1] * 2;

	CHitBoxSet* hitbox_set = target->GetHitboxSet(0);
	if (hitbox_set == nullptr)
		return false;

	auto& hitboxes = hitbox_set->GetHitboxes();

	if (hitboxes.m_Size <= 0 || hitboxes.m_Size > 0xFFFF)
		return false;

	auto hitbox_transform = reinterpret_cast<CTransform*>(I::MemAlloc->Alloc(sizeof(CTransform) * HITBOX_MAX));
	if (hitbox_transform == nullptr)
		return false;

	if (target->HitboxToWorldTransform(hitbox_set, hitbox_transform) == 0)
		return false;

	for (int i = 0; i < HITBOX_MAX; i++)
	{
		CHitBox* hitbox = &hitboxes.m_Data[i];
		if (hitbox == nullptr || hitbox->m_name == nullptr)
			continue;

		float radius = hitbox->m_flShapeRadius * (multiPoint / 100.f);
		CTransform transform = hitbox_transform[i];
		Vector_t min_bounds = hitbox->m_vMinBounds - radius;
		Vector_t max_bounds = hitbox->m_vMaxBounds + radius;

		Matrix3x4_t hitbox_matrix = hitbox_transform[i].ToMatrix3x4();
		Vector_t mins = min_bounds.Transform(hitbox_matrix);
		Vector_t maxs = max_bounds.Transform(hitbox_matrix);
		Vector_t hitbox_pos = (mins + maxs) * 0.5f;

		float u = MATH::fnRandomFloat(0.f, 1.f),
			  v = MATH::fnRandomFloat(0.f, 1.f),
			  w = MATH::fnRandomFloat(0.f, 1.f),
			norm = std::sqrtf(u * u + v * v + w * w);

		u /= norm;
		v /= norm;
		w /= norm;

		Vector_t scanPos = Vector_t(
			hitbox_pos.x + radius * u,
			hitbox_pos.y + radius * v,
			hitbox_pos.z + radius * w
		);

		F::PENETRATION::c_auto_wall AutoWall{};
		F::PENETRATION::c_auto_wall::data_t hitData{};
		AutoWall.pen(hitData, shootPos, scanPos, pLocalPawn, target, vData);
		if (!hitData.m_can_hit)
			continue;

		if (minDamage > hitData.m_dmg)
			continue;
		result->damage = hitData.m_dmg;

		QAngle_t aimAngle = MATH::CalculateAngles(shootPos, scanPos);
		if (C_GET(bool, Vars.bHideshots))
			aimAngle = QAngle_t(180 - aimAngle.x, MATH::NormalizeYaw(aimAngle.y + 180), aimAngle.z);
		aimAngle -= aimPunch;

		if (variables.bAutoScope && vData->GetWeaponType() == WEAPONTYPE_SNIPER_RIFLE && weapon->GetZoomLevel() < 1)
		{
			pCmd->nButtons.nValue |= IN_SECOND_ATTACK;
			pBaseCmd->PressButton(IN_SECOND_ATTACK);
			return false; // Can't hit, must zoom first.
		}

		if (!HitChance(pLocalPawn, target, scanPos, hitChance, i, &result->hitChance))
		{
			if (variables.bAutoStop)
				AutoStop(pLocalPawn, weapon, pCmd, pBaseCmd);
			continue;
		}

		result->aimAngle = aimAngle;
		result->LCSimTime = target->GetSimulationTime();
		result->LCTick = TIME_TO_TICKS(result->LCSimTime);
		result->canHit = true;
		result->hitbox = i;
		return true;
	}

	I::MemAlloc->Free(hitbox_transform);

	/*skeleton->CalculateWorldSpaceBones(EBoneFlags::FLAG_HITBOX);
	auto& model_state = skeleton->GetModelState();
	CModel* model = model_state.GetModel();
	int num_bones = skeleton->nBoneCount;
	Matrix2x4_t* bones = skeleton->pBoneCache;
	for (auto i = 0u; i < num_bones; i++)
	{
		if (model->GetBoneFlags(i) & EBoneFlags::FLAG_HITBOX)
		{
			Vector_t boneOrigin = bones->GetOrigin(i);
			F::PENETRATION::c_auto_wall AutoWall{};
			F::PENETRATION::c_auto_wall::data_t hitData{};
			AutoWall.pen(hitData, shootPos, boneOrigin, pLocalPawn, target, vData);
			if (!hitData.m_can_hit)
				continue;

			if (minDamage > hitData.m_dmg)
				continue;

			QAngle_t aimAngle = MATH::CalculateAngles(shootPos, boneOrigin);
			if (C_GET(bool, Vars.bHideshots))
				aimAngle = QAngle_t(180 - aimAngle.x, MATH::NormalizeYaw(aimAngle.y + 180), aimAngle.z);
			aimAngle -= aimPunch;

			if (variables.bAutoScope && vData->GetWeaponType() == WEAPONTYPE_SNIPER_RIFLE && weapon->GetZoomLevel() < 1)
			{
				pCmd->nButtons.nValue |= IN_SECOND_ATTACK;
				pBaseCmd->PressButton(IN_SECOND_ATTACK);
				return false; // Can't hit, must zoom first.
			}

			unsigned int hitbox = HitboxBoneToHitboxId(i);
			if (!HitChance(pLocalPawn, target, boneOrigin, hitChance, hitbox))
			{
				if (variables.bAutoStop)
					AutoStop(pLocalPawn, weapon, pCmd, pBaseCmd);
				continue;
			}

			result->aimAngle = aimAngle;
			result->LCSimTime = target->GetSimulationTime();
			result->LCTick = TIME_TO_TICKS(result->LCSimTime);
			result->canHit = true;
			result->hitbox = hitbox;
			return true;
		}
	}*/

	result->canHit = false;
	return false;
}

void F::RAGEBOT::RAGE::AutoRevolver(C_CSPlayerPawn* pLocalPawn, CUserCmd* pCmd)
{
	C_CSWeaponBaseGun* pWeapon = pLocalPawn->GetCurrentWeapon();
	if (pWeapon == nullptr)
		return;

	C_AttributeContainer* attribs = pWeapon->GetAttributeManager();
	if (attribs == nullptr)
		return;

	C_EconItemView* item = attribs->GetItem();
	if (item == nullptr)
		return;

	static int ticks = 0;
	static int oldTicks = 0;
	if (item->GetItemDefinitionIndex() != WEAPON_R8_REVOLVER || !pLocalPawn->CanAttack())
	{
		ticks = 0;
		oldTicks = I::GlobalVars->nCurrentTick;
		return;
	}

	int currentTick = I::GlobalVars->nCurrentTick;
	if (currentTick != oldTicks)
	{
		ticks++;
		oldTicks = currentTick;
	}

	if (ticks < 6)
		pCmd->nButtons.nValue |= IN_ATTACK;
	else if (ticks < 20)
		pCmd->nButtons.nValue |= IN_SECOND_ATTACK;

	if (ticks > 20)
		ticks = 0;
}

void F::RAGEBOT::RAGE::OnMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController, C_CSPlayerPawn* pLocalPawn)
{
	if (!C_GET(bool, Vars.bRageEnable))
		return;

	AutoRevolver(pLocalPawn, pCmd);
	PickScanTarget(pLocalController, pLocalPawn);

	C_CSPlayerPawn* target = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(scanTarget.entity);
	if (target == nullptr || target->GetHealth() < 1)
	{
		PickScanTarget(pLocalController, pLocalPawn);
		if (!scanTarget.entity.IsValid())
			return;
		target = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(scanTarget.entity);
	}

	HitScanResult result{};
	result.target = target;
	bool canHit = HitScan(&result, pCmd, pBaseCmd, pLocalController, pLocalPawn);
	if (!canHit)
	{
		result = HitScanResult{}; // Make a new result object
		result.target = target;
		F::LAGCOMP::Apply(target, 0); // Apply the latest record saved
		canHit = HitScan(&result, pCmd, pBaseCmd, pLocalController, pLocalPawn); // Scan again
		F::LAGCOMP::Restore(target); // Restore to backup
		if (!canHit)
			return;
	}

	if (!pLocalPawn->CanAttack())
		return;

	CBaseHandle ControllerHandle = result.target->GetControllerHandle();
	auto TargetController = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(ControllerHandle);
	if (TargetController == nullptr)
		return;

	SDK::fnConColorMsg(Color_t(0.f, 0.f, 1.f), "RAGEBOT DEBUG START\n");
	std::ostringstream debugMessage{};
	debugMessage << "Firing a shot at " << TargetController->GetPlayerName() << " at tick " << I::GlobalVars->nCurrentTick << " into tick " << result.LCTick << " (" << result.LCSimTime << " simtime)\n";
	debugMessage << "Hit-chance: " << result.hitChance * 100 << "%% \\ Expected damage: " << result.damage << " \\ At hitbox: " << result.hitbox << "\n";

	QAngle_t aimAngle = result.aimAngle;
	debugMessage << "Shot angle: (" << aimAngle.x << ", " << aimAngle.y << ", " << aimAngle.z << ")\n";
	if (C_GET(bool, Vars.bHideshots))
		aimAngle.x -= 40;
	pBaseCmd->pViewAngles->angValue = aimAngle;
	pBaseCmd->pViewAngles->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
	pBaseCmd->PressButton(IN_ATTACK);
	debugMessage << "Sub-tick attack fraction: " << I::GlobalVars->flTickFraction1 << "\n";
	pCmd->nButtons.nValue |= IN_ATTACK;

	int originalTickCount = -1;
	if (pCmd->csgoUserCmd.inputHistoryField.pRep->nAllocatedSize > 0)
		originalTickCount = pCmd->csgoUserCmd.inputHistoryField.pRep->tElements[0]->nPlayerTickCount;

	pCmd->ClearInputHistory();

	int tick = result.LCTick;
	float frac = 0.f;
	CCSGOInputHistoryEntryPB* pInputEntry = pCmd->AddInputHistoryEntry();
	if (pInputEntry == nullptr)
		return;

	CMsgQAngle* viewAngle = pInputEntry->CreateQAngle();
	viewAngle->angValue = aimAngle;
	if (C_GET(bool, Vars.bHideshots))
		viewAngle->angValue.x += 40;
	pInputEntry->pViewAngles = viewAngle;
	pInputEntry->pViewAngles->nCachedBits = 64424509447;

	CCSGOInterpolationInfoPB_CL* cl_interp = pInputEntry->CreateInterpCL();
	cl_interp->flFraction = frac;
	cl_interp->nCachedBits = 21474836481;
	pInputEntry->cl_interp = cl_interp;

	CCSGOInterpolationInfoPB* sv_interp0 = pInputEntry->CreateInterp();
	sv_interp0->flFraction = 0.f;
	sv_interp0->nSrcTick = tick;
	sv_interp0->nDstTick = tick + 1;
	sv_interp0->nCachedBits = 47244640263;
	pInputEntry->sv_interp0 = sv_interp0;

	CCSGOInterpolationInfoPB* sv_interp1 = pInputEntry->CreateInterp();
	sv_interp1->flFraction = 0.f;
	sv_interp1->nSrcTick = tick + 1;
	sv_interp1->nDstTick = tick + 2;
	sv_interp1->nCachedBits = 47244640263;
	pInputEntry->sv_interp1 = sv_interp1;

	CCSGOInterpolationInfoPB* player_interp = pInputEntry->CreateInterp();
	player_interp->flFraction = MATH::Max(cl_interp->flFraction - 0.05f, 0.f);
	player_interp->nSrcTick = tick + 1;
	player_interp->nDstTick = tick + 2;
	player_interp->nCachedBits = 47244640263;
	pInputEntry->player_interp = player_interp;

	pInputEntry->nRenderTickCount = tick + 2;
	pInputEntry->flRenderTickFraction = frac;

	pInputEntry->nPlayerTickCount = originalTickCount < 0 ? I::GlobalVars->nCurrentTick : originalTickCount;
	pInputEntry->flPlayerTickFraction = I::GlobalVars->flTickFraction1;
	pInputEntry->nCachedBits = 339302424095;

	debugMessage << "Finished constructing input entry.\nLC Delta: " << pInputEntry->nPlayerTickCount - tick << "\n";

	pCmd->csgoUserCmd.nAttack1StartHistoryIndex = 0;
	pCmd->csgoUserCmd.nAttack3StartHistoryIndex = 0;
	pCmd->csgoUserCmd.inputHistoryField.add(pInputEntry);

	std::string string = debugMessage.str();
	SDK::fnMsg(string.c_str());

	SDK::fnConColorMsg(Color_t(0.f, 0.f, 1.f), "RAGEBOT DEBUG END\n");
}
