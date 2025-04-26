#include "lagcomp.h"
#include "../../sdk/entity.h"
#include <unordered_map>
#include <deque>
#define ARRAY_RECORDS

#ifdef ARRAY_RECORDS
std::unordered_map<C_CSPlayerPawn*, std::deque<BacktrackRecord*>*> backtrackRecords{};
#else
std::unordered_map<C_CSPlayerPawn*, BacktrackRecord*> backtrackRecords{};
#endif
std::unordered_map<C_CSPlayerPawn*, BackupState*> backups{};

void F::LAGCOMP::Create(C_CSPlayerPawn* pawn)
{
	// Sanity checks
	if (pawn == nullptr)
		return;

	CGameSceneNode* node = pawn->GetGameSceneNode();
	if (node == nullptr)
		return;

	CSkeletonInstance* skeleton = node->GetSkeletonInstance();
	if (skeleton == nullptr)
		return;

	#ifdef ARRAY_RECORDS
	// If there's no backtrack records map for this player made yet, make one
	std::deque<BacktrackRecord*>* records = nullptr;
	if (!backtrackRecords.contains(pawn))
	{
		records = new std::deque<BacktrackRecord*>();
		backtrackRecords.insert({ pawn, records });
	}
	else
		records = backtrackRecords.at(pawn);

	if (records == nullptr)
		return;

	// Remove all invalid records.
	for (auto it = records->begin(); it != records->end();)
	{
		BacktrackRecord* record = *it;
		if (!record->IsRecordValid())
			it = records->erase(it);
		else
			++it;
	}
	#else
	BacktrackRecord* records = nullptr;
	if (backtrackRecords.contains(pawn))
	{
		records = backtrackRecords[pawn];
		if (records->IsRecordValid())
			return;
	}
	else
		backtrackRecords.insert({ pawn, nullptr });
	#endif

	int boneCount = skeleton->nBoneCount;
	float simTime = pawn->GetSimulationTime();
	auto record = new BacktrackRecord();
	auto bone_array = reinterpret_cast<Matrix2x4_t*>(I::MemAlloc->Alloc(sizeof(Matrix2x4_t) * boneCount));
	CRT::MemoryCopy(bone_array, skeleton->pBoneCache, sizeof(Matrix2x4_t) * boneCount);
	record->pBones = bone_array;
	record->Origin = node->GetOrigin();
	record->absOrigin = node->GetAbsOrigin();
	record->Rotation = node->GetAngleRotation();
	record->absRotation = node->GetAbsAngleRotation();
	record->eyeAngles = pawn->GetEyeAngles();
	record->nBoneCount = boneCount;
	record->fSimTime = simTime;
	#ifdef ARRAY_RECORDS
	records->push_back(record);
	#else
	backtrackRecords[pawn] = record;
	delete records;
	#endif
}

void F::LAGCOMP::Apply(C_CSPlayerPawn* pawn, size_t pos)
{
	if (pawn == nullptr)
		return;

	if (!backtrackRecords.contains(pawn))
		return;

	if (backups.contains(pawn))
		F::LAGCOMP::Restore(pawn);

	CGameSceneNode* node = pawn->GetGameSceneNode();
	if (node == nullptr)
		return;

	CSkeletonInstance* skeleton = node->GetSkeletonInstance();
	if (skeleton == nullptr)
		return;

	#ifdef ARRAY_RECORDS
	std::deque<BacktrackRecord*>* records = backtrackRecords.at(pawn);
	#else
	BacktrackRecord* records = backtrackRecords.at(pawn);
	#endif

	if (records == nullptr)
		return;

	#ifdef ARRAY_RECORDS
	if (records->empty())
		return;
	#endif

	BackupState* state = new BackupState();
	state->Origin = node->GetOrigin();
	state->absOrigin = node->GetAbsOrigin();
	state->Rotation = node->GetAngleRotation();
	state->absRotation = node->GetAbsAngleRotation();
	state->eyeAngles = pawn->GetEyeAngles();
	state->nBoneCount = skeleton->nBoneCount;
	state->fSimTime = pawn->GetSimulationTime();

	auto pBackupBones = reinterpret_cast<Matrix2x4_t*>(I::MemAlloc->Alloc(sizeof(Matrix2x4_t) * state->nBoneCount));
	CRT::MemoryCopy(pBackupBones, skeleton->pBoneCache, sizeof(Matrix2x4_t) * state->nBoneCount);
	state->pBones = pBackupBones;
	backups.insert({ pawn, state });

	#ifdef ARRAY_RECORDS
	pos = MATH::Clamp<size_t>(pos, 0, records->size() - 1);

	BacktrackRecord* record = records->at(pos);
	#else
	BacktrackRecord* record = records;
	#endif
	node->GetOrigin() = record->Origin;
	node->GetAbsOrigin() = record->absOrigin;
	node->GetAngleRotation() = record->Rotation;
	node->GetAbsAngleRotation() = record->absRotation;
	if (record->nBoneCount > skeleton->nBoneCount)
	{
		auto pBones = reinterpret_cast<Matrix2x4_t*>(I::MemAlloc->ReAlloc(skeleton->pBoneCache, sizeof(Matrix2x4_t) * record->nBoneCount));
		CRT::MemoryCopy(pBones, record->pBones, sizeof(Matrix2x4_t) * record->nBoneCount);
		skeleton->pBoneCache = pBones;
	}
	else
		CRT::MemoryCopy(skeleton->pBoneCache, record->pBones, sizeof(Matrix2x4_t) * record->nBoneCount);

	skeleton->nBoneCount = record->nBoneCount;
	pawn->GetSimulationTime() = record->fSimTime;
}

void F::LAGCOMP::Restore(C_CSPlayerPawn* pawn)
{
	if (pawn == nullptr)
		return;

	if (!backups.contains(pawn))
		return;

	CGameSceneNode* node = pawn->GetGameSceneNode();
	if (node == nullptr)
		return;

	CSkeletonInstance* skeleton = node->GetSkeletonInstance();
	if (skeleton == nullptr)
		return;

	BackupState* state = backups.at(pawn);
	if (state == nullptr)
		return;

	node->GetOrigin() = state->Origin;
	node->GetAbsOrigin() = state->absOrigin;
	node->GetAngleRotation() = state->Rotation;
	node->GetAbsAngleRotation() = state->absRotation;
	pawn->GetEyeAngles() = state->eyeAngles;
	if (state->nBoneCount > skeleton->nBoneCount)
	{
		auto pBones = reinterpret_cast<Matrix2x4_t*>(I::MemAlloc->ReAlloc(skeleton->pBoneCache, sizeof(Matrix2x4_t) * state->nBoneCount));
		CRT::MemoryCopy(pBones, state->pBones, sizeof(Matrix2x4_t) * state->nBoneCount);
		skeleton->pBoneCache = pBones;
	}
	else
		CRT::MemoryCopy(skeleton->pBoneCache, state->pBones, sizeof(Matrix2x4_t) * state->nBoneCount);

	skeleton->nBoneCount = state->nBoneCount;
	pawn->GetSimulationTime() = state->fSimTime;
	backups.erase(pawn);
	delete state;
}

void F::LAGCOMP::Destroy(C_CSPlayerPawn* pawn)
{
	if (pawn == nullptr)
		return;

	if (backups.contains(pawn))
		F::LAGCOMP::Restore(pawn);

	if (backtrackRecords.contains(pawn))
	{
		#ifdef ARRAY_RECORDS
		std::deque<BacktrackRecord*>* records = backtrackRecords.at(pawn);
		for (BacktrackRecord* record : *records) // Free elements
			delete record;
		delete records; // Free vector
		#else
		delete backtrackRecords.at(pawn);
		#endif
		backtrackRecords.erase(pawn);
	}
	// Basically done?
}
