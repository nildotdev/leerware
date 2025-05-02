#include "rage.h"
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>

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

class ThreadSafeHitScanResult {
public:
	std::mutex mutex;
	HitScanResult result;
	std::atomic<bool> ready{false};
	std::atomic<bool> processing{false};
};

HitScanTarget scanTarget{};
ThreadSafeHitScanResult g_ThreadedResult;

static bool g_AutoStopActive = false;
static bool g_JustFired = false;
static int g_LastShotTick = 0;
static bool g_IsShooting = false;

void ThreadedHitScan(C_CSPlayerPawn* pLocalPawn, C_CSPlayerPawn* target, CCSWeaponBaseVData* vData, 
                     int multiPoint, int hitChance, int minDamage, Vector_t shootPos, QAngle_t aimPunch);

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
	scanTarget.entity = CBaseHandle{};
	float flDistRecord = INFINITY;
	float flFovRecord = 180.f;
	int lowestHealth = 100;

	Vector_t localPos = pLocalPawn->GetSceneOrigin();
	Vector_t localEyePos = pLocalPawn->GetEyePosition();
	
	int selectionMode = 1;

	for (CCSPlayerController* pPlayer : SDK::PlayerControllers)
	{
		if (pPlayer->IsLocalPlayerController())
			continue;

		if (!pPlayer->IsPawnAlive())
			continue;

		C_CSPlayerPawn* pPawn = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer->GetPawnHandle());
		if (pPawn == nullptr)
			continue;

		if (!pLocalPawn->IsOtherEnemy(pPawn))
			continue;

		Vector_t enemyPos = pPawn->GetSceneOrigin();
		
		float distance = localPos.DistTo(enemyPos);
		QAngle_t aimAngle = MATH::CalculateAngles(localEyePos, enemyPos);
		float fov = MATH::CalculateFOVDistance(I::Input->vecViewAngle, aimAngle);
		int health = pPawn->GetHealth();
		
		bool isNewTarget = false;
		
		switch (selectionMode)
		{
		case 0:
			if (distance < flDistRecord) {
				flDistRecord = distance;
				isNewTarget = true;
			}
			break;
			
		case 1:
			if (fov < flFovRecord) {
				flFovRecord = fov;
				isNewTarget = true;
			}
			break;
			
		case 2:
			if (health < lowestHealth && health > 0) {
				lowestHealth = health;
				isNewTarget = true;
			}
			break;
			
		default:
			if (fov < flFovRecord) {
				flFovRecord = fov;
				isNewTarget = true;
			}
			break;
		}
		
		if (isNewTarget) {
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
	auto wep_info = weapon->GetWeaponVData();
	if (!wep_info)
		return {};

	item_def_index = wep_info->GetWeaponName();
	recoil_index = weapon->GetRecoilIndex();

	MATH::fnRandomSeed((seed & 0xff) + 1);

	r1 = MATH::fnRandomFloat(0.f, 1.f);
	r2 = MATH::fnRandomFloat(0.f, MATH::_PI * 2);
	r3 = MATH::fnRandomFloat(0.f, 1.f);
	r4 = MATH::fnRandomFloat(0.f, MATH::_PI * 2);

	if (item_def_index == CS_XOR("weapon_revoler") && revolver2)
	{
		r1 = 1.f - (r1 * r1);
		r3 = 1.f - (r3 * r3);
	}

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

	c1 = std::cos(r2);
	c2 = std::cos(r4);
	s1 = std::sin(r2);
	s2 = std::sin(r4);

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
	
	float maxSpeed = WeaponVData->GetMaxSpeed()[fireType] * 0.03f;
	
	F::MISC::MOVEMENT::LimitSpeed(pBaseCmd, pLocal, maxSpeed);
	g_AutoStopActive = true;
}

float currentFraction = 0.f;
constexpr unsigned int HITBOX_MAX = 18;

void ThreadedHitScan(C_CSPlayerPawn* pLocalPawn, C_CSPlayerPawn* target, CCSWeaponBaseVData* vData, 
                    int multiPoint, int hitChance, int minDamage, Vector_t shootPos, QAngle_t aimPunch)
{
	g_ThreadedResult.processing = true;
	
	HitScanResult localResult{};
	localResult.target = target;
	localResult.canHit = false;
	
	if (!pLocalPawn || !target || !vData) {
		g_ThreadedResult.processing = false;
		return;
	}
	
	CGameSceneNode* node = target->GetGameSceneNode();
	if (node == nullptr) {
		g_ThreadedResult.processing = false;
		return;
	}

	CSkeletonInstance* skeleton = node->GetSkeletonInstance();
	if (skeleton == nullptr) {
		g_ThreadedResult.processing = false;
		return;
	}

	CHitBoxSet* hitbox_set = target->GetHitboxSet(0);
	if (hitbox_set == nullptr) {
		g_ThreadedResult.processing = false;
		return;
	}

	auto& hitboxes = hitbox_set->GetHitboxes();
	if (hitboxes.m_Size <= 0 || hitboxes.m_Size > 0xFFFF) {
		g_ThreadedResult.processing = false;
		return;
	}

	auto hitbox_transform = reinterpret_cast<CTransform*>(I::MemAlloc->Alloc(sizeof(CTransform) * HITBOX_MAX));
	if (hitbox_transform == nullptr) {
		g_ThreadedResult.processing = false;
		return;
	}

	if (target->HitboxToWorldTransform(hitbox_set, hitbox_transform) == 0) {
		I::MemAlloc->Free(hitbox_transform);
		g_ThreadedResult.processing = false;
		return;
	}

	int priorityHitboxes[HITBOX_MAX];
	int priorityCount = 0;
    
	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_HEAD;
	
	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_CHEST;
	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_CHEST1;

	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_STOMACH;
	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_PELVIS;
	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_PELVIS1;

	bool ignoreLimbs = target->GetVecVelocity().Length2D() > 71.f;
	if (!ignoreLimbs) {
		priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_LEFTUPPERARM;
		priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_RIGHTUPPERARM;
		priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_LEFTLOWERARM;
		priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_RIGHTLOWERARM;
	}

	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_LEFTUPPERLEG;
	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_RIGHTUPPERLEG;
	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_LEFTLOWERLEG;
	priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_RIGHTLOWERLEG;

	if (!ignoreLimbs) {
		priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_LEFTTOE;
		priorityHitboxes[priorityCount++] = EHitboxes::HITBOX_RIGHTTOE;
	}

	Vector_t bestPoint;
	QAngle_t bestAngle;
	float bestDamage = 0.f;
	float bestHitChance = 0.f;
	int bestHitbox = -1;
	bool foundPoint = false;

	for (int p = 0; p < priorityCount; p++) {
		int i = priorityHitboxes[p];
		
		if (i >= hitboxes.m_Size)
			continue;
			
		CHitBox* hitbox = &hitboxes.m_Data[i];
		if (hitbox == nullptr || hitbox->m_name == nullptr)
			continue;

		float radius = hitbox->m_flShapeRadius * (multiPoint / 100.f);
		CTransform transform = hitbox_transform[i];
		Vector_t min_bounds = hitbox->m_vMinBounds;
		Vector_t max_bounds = hitbox->m_vMaxBounds;

		Matrix3x4_t hitbox_matrix = hitbox_transform[i].ToMatrix3x4();
		Vector_t mins = min_bounds.Transform(hitbox_matrix);
		Vector_t maxs = max_bounds.Transform(hitbox_matrix);
		Vector_t center = (mins + maxs) * 0.5f;

		Vector_t points[8];
		int pointCount = 0;

		points[pointCount++] = center;

		bool reducedPointCount = p > 2 && SDK::PlayerControllers.size() > 3;

		if (i == EHitboxes::HITBOX_HEAD && multiPoint > 60 && !reducedPointCount) {
			points[pointCount++] = Vector_t(center.x, center.y, maxs.z);
			
			if (multiPoint > 80) {
				points[pointCount++] = Vector_t(maxs.x, center.y, center.z);
				points[pointCount++] = Vector_t(mins.x, center.y, center.z);
				points[pointCount++] = Vector_t(center.x, maxs.y, center.z);
			}
		}
		else if ((i == EHitboxes::HITBOX_CHEST || i == EHitboxes::HITBOX_CHEST1) && multiPoint > 60 && !reducedPointCount) {
			points[pointCount++] = Vector_t(center.x, maxs.y, center.z);
		}
		else if ((i == EHitboxes::HITBOX_STOMACH || i == EHitboxes::HITBOX_PELVIS || i == EHitboxes::HITBOX_PELVIS1) && multiPoint > 60 && !reducedPointCount) {
			points[pointCount++] = Vector_t(center.x, maxs.y, center.z);
		}
		else if ((i == EHitboxes::HITBOX_LEFTUPPERLEG || i == EHitboxes::HITBOX_RIGHTUPPERLEG || 
				 i == EHitboxes::HITBOX_LEFTLOWERLEG || i == EHitboxes::HITBOX_RIGHTLOWERLEG) && multiPoint > 60 && !reducedPointCount) {
			points[pointCount++] = Vector_t(center.x, center.y, maxs.z);
		}
		
		if (radius > 3.0f && pointCount < 8 && !reducedPointCount) {
			float u = MATH::fnRandomFloat(0.f, 1.f);
			float v = MATH::fnRandomFloat(0.f, 1.f);
			float w = MATH::fnRandomFloat(0.f, 1.f);
			float norm = std::sqrtf(u * u + v * v + w * w);

			u /= norm;
			v /= norm;
			w /= norm;

			points[pointCount++] = Vector_t(
				center.x + radius * u * 0.8f,
				center.y + radius * v * 0.8f,
				center.z + radius * w * 0.8f
			);
		}

		for (int j = 0; j < pointCount; j++) {
			Vector_t scanPos = points[j];
			
			F::PENETRATION::c_auto_wall AutoWall{};
			F::PENETRATION::c_auto_wall::data_t hitData{};
			AutoWall.pen(hitData, shootPos, scanPos, pLocalPawn, target, vData);
			
			if (!hitData.m_can_hit)
				continue;
				
			if (minDamage > hitData.m_dmg)
				continue;
				
			QAngle_t aimAngle = MATH::CalculateAngles(shootPos, scanPos);
			if (C_GET(bool, Vars.bHideshots))
				aimAngle = QAngle_t(180 - aimAngle.x, MATH::NormalizeYaw(aimAngle.y + 180), aimAngle.z);
			aimAngle -= aimPunch;
			
			float outChance = 0.f;
			if (!F::RAGEBOT::RAGE::HitChance(pLocalPawn, target, scanPos, hitChance, i, &outChance)) {
				continue;
			}
			
			bool shouldSelect = false;
			
			if (!foundPoint) {
				shouldSelect = true;
			}
			else if (p < bestHitbox && hitData.m_dmg >= minDamage) {
				shouldSelect = true;
			}
			else if (i == bestHitbox && hitData.m_dmg > bestDamage) {
				shouldSelect = true;
			}
			else if (hitData.m_dmg >= target->GetHealth() && bestDamage < target->GetHealth()) {
				shouldSelect = true;
			}
			
			if (shouldSelect) {
				bestPoint = scanPos;
				bestAngle = aimAngle;
				bestDamage = hitData.m_dmg;
				bestHitChance = outChance;
				bestHitbox = i;
				foundPoint = true;
				
				if (p <= 2 && hitData.m_dmg >= target->GetHealth()) {
					break;
				}
			}
		}
		
		if (foundPoint && p <= 2 && bestDamage >= target->GetHealth()) {
			break;
		}
	}

	I::MemAlloc->Free(hitbox_transform);
	
	if (foundPoint) {
		localResult.aimAngle = bestAngle;
		localResult.LCSimTime = target->GetSimulationTime();
		localResult.LCTick = TIME_TO_TICKS(localResult.LCSimTime);
		localResult.canHit = true;
		localResult.hitbox = bestHitbox;
		localResult.hitChance = bestHitChance;
		localResult.damage = bestDamage;
	}

	{
		std::lock_guard<std::mutex> lock(g_ThreadedResult.mutex);
		g_ThreadedResult.result = localResult;
		g_ThreadedResult.ready = true;
	}
	
	g_ThreadedResult.processing = false;
}

// Modified HitScan function to use threading
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

	if (g_ThreadedResult.ready && g_ThreadedResult.result.target == target) {
		std::lock_guard<std::mutex> lock(g_ThreadedResult.mutex);
		*result = g_ThreadedResult.result;
		
		if (result->canHit && variables.bAutoScope && vData->GetWeaponType() == WEAPONTYPE_SNIPER_RIFLE && weapon->GetZoomLevel() < 1) {
			pCmd->nButtons.nValue |= IN_SECOND_ATTACK;
			pBaseCmd->PressButton(IN_SECOND_ATTACK);
			return false;
		}
		
		if (!result->canHit && variables.bAutoStop) {
			AutoStop(pLocalPawn, weapon, pCmd, pBaseCmd);
		}
		
		g_ThreadedResult.ready = false;
		return result->canHit;
	}

	if (!g_ThreadedResult.processing) {
		g_ThreadedResult.ready = false;
		
		std::thread scanThread(ThreadedHitScan, pLocalPawn, target, vData, 
			multiPoint, hitChance, minDamage, shootPos, aimPunch);
			
		scanThread.detach();
	}
	
	if (g_ThreadedResult.result.target == target && g_ThreadedResult.result.canHit) {
		std::lock_guard<std::mutex> lock(g_ThreadedResult.mutex);
		*result = g_ThreadedResult.result;
		return true;
	}
	
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
	{
		g_IsShooting = false;  // Reset shooting state if rage is disabled
		return;
	}

	// Clear AutoStop if we just fired in previous tick
	int currentTick = I::GlobalVars->nCurrentTick;
	if (g_JustFired && currentTick > g_LastShotTick)
	{
		g_AutoStopActive = false;
		g_JustFired = false;
	}
	
	// Check if player has anti-aim enabled - don't interfere if it is
	if (C_GET(bool, Vars.bAntiAimEnable) && pBaseCmd->pViewAngles)
	{
		// Anti-aim is active, don't modify view angles from rage
		g_IsShooting = false;
		return;
	}
	
	AutoRevolver(pLocalPawn, pCmd);
	
	// Check if player can attack
	if (!pLocalPawn->CanAttack())
	{
		// Don't modify view angles if we can't shoot
		g_IsShooting = false;
		return;
	}
	
	// Get weapon
	C_CSWeaponBaseGun* weapon = pLocalPawn->GetCurrentWeapon();
	if (weapon == nullptr)
	{
		g_IsShooting = false;
		return;
	}
	
	PickScanTarget(pLocalController, pLocalPawn);

	C_CSPlayerPawn* target = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(scanTarget.entity);
	if (target == nullptr || target->GetHealth() < 1)
	{
		PickScanTarget(pLocalController, pLocalPawn);
		if (!scanTarget.entity.IsValid())
		{
			g_IsShooting = false;  // Stop shooting if no valid target
			return;
		}
		target = I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(scanTarget.entity);
	}

	HitScanResult result{};
	result.target = target;
	
	RageBotVars_t variables = C_GET(RageBotVars_t, Vars.varsGlobal);
	
	auto weaponData = weapon->GetWeaponVData();
	if (!weaponData)
	{
		g_IsShooting = false;
		return;
	}
	
	C_AttributeContainer* attribs = weapon->GetAttributeManager();
	if (attribs)
	{
		C_EconItemView* item = attribs->GetItem();
		if (item)
		{
			short currentWeapon = item->GetItemDefinitionIndex();
			switch (currentWeapon)
			{
			case WEAPON_SSG_08:
				variables = C_GET(RageBotVars_t, Vars.varsScout);
				break;
			case WEAPON_G3SG1:
			case WEAPON_SCAR_20:
				variables = C_GET(RageBotVars_t, Vars.varsAuto);
				break;
			case WEAPON_AWP:
				variables = C_GET(RageBotVars_t, Vars.varsAWP);
				break;
			case WEAPON_DESERT_EAGLE:
				variables = C_GET(RageBotVars_t, Vars.varsDeagle);
				break;
			case WEAPON_R8_REVOLVER:
				variables = C_GET(RageBotVars_t, Vars.varsR8);
				break;
			default:
				if (weaponData->GetWeaponType() == WEAPONTYPE_PISTOL)
					variables = C_GET(RageBotVars_t, Vars.varsPistols);
				break;
			}
		}
	}

	bool canHit = HitScan(&result, pCmd, pBaseCmd, pLocalController, pLocalPawn);
	
	if (canHit && variables.bAutoStop && !g_AutoStopActive)
	{
		AutoStop(pLocalPawn, weapon, pCmd, pBaseCmd);
	}
	
	if (!canHit)
	{
		result = HitScanResult{};
		result.target = target;
		F::LAGCOMP::Apply(target, 0);
		canHit = HitScan(&result, pCmd, pBaseCmd, pLocalController, pLocalPawn);
		F::LAGCOMP::Restore(target);
		
		if (canHit && variables.bAutoStop && !g_AutoStopActive)
		{
			AutoStop(pLocalPawn, weapon, pCmd, pBaseCmd);
		}
			
		if (!canHit)
		{
			g_IsShooting = false;
			return;
		}
	}

	if (weapon->GetNextPrimaryAttackTick() > I::GlobalVars->nCurrentTick && g_IsShooting == false)
	{
		return;
	}

	CBaseHandle ControllerHandle = result.target->GetControllerHandle();
	auto TargetController = I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(ControllerHandle);
	if (TargetController == nullptr)
	{
		g_IsShooting = false;
		return;
	}

	SDK::fnConColorMsg(Color_t(0.f, 0.f, 1.f), "RAGEBOT DEBUG START\n");
	std::ostringstream debugMessage{};
	debugMessage << "Firing a shot at " << TargetController->GetPlayerName() << " at tick " << I::GlobalVars->nCurrentTick << " into tick " << result.LCTick << " (" << result.LCSimTime << " simtime)\n";
	debugMessage << "Hit-chance: " << result.hitChance * 100 << "%% \\ Expected damage: " << result.damage << " \\ At hitbox: " << result.hitbox << "\n";

	QAngle_t aimAngle = result.aimAngle;
	debugMessage << "Shot angle: (" << aimAngle.x << ", " << aimAngle.y << ", " << aimAngle.z << ")\n";
	
	if (C_GET(bool, Vars.bHideshots))
		aimAngle.x -= 40;
	
	if (canHit && pBaseCmd->pViewAngles)
	{
		pBaseCmd->pViewAngles->angValue = aimAngle;
		pBaseCmd->pViewAngles->SetBits(EBaseCmdBits::BASE_BITS_VIEWANGLES);
		
		if (!g_IsShooting)
		{
			debugMessage << "Starting to shoot...\n";
			pBaseCmd->PressButton(IN_ATTACK);
			g_IsShooting = true;
			
			g_JustFired = true;
			g_LastShotTick = currentTick;
		}
		else
		{
			debugMessage << "Continuing to shoot...\n";
		}
		
		pCmd->nButtons.nValue |= IN_ATTACK;
	}
	else
	{
		g_IsShooting = false;
	}
	
	debugMessage << "Sub-tick attack fraction: " << I::GlobalVars->flTickFraction1 << "\n";

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
