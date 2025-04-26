#pragma once
#include "../../core/interfaces.h"
#include "../../utilities/fnv1a.h"
#include "../../sdk/datatypes/matrix.h"
#include "../../sdk/interfaces/ienginecvar.h"
#include "../../core/variables.h"
#include "../../core/convars.h"

class BacktrackRecord
{
public:
	Matrix2x4_t* pBones;
	CNetworkOriginCellCoordQuantizedVector Origin;
	Vector_t absOrigin;
	QAngle_t Rotation;
	QAngle_t absRotation;
	QAngle_t eyeAngles;
	int nBoneCount;
	float fSimTime;

	BacktrackRecord()
	{
		pBones = nullptr;
		Origin = CNetworkOriginCellCoordQuantizedVector{};
		absOrigin = Vector_t{};
		Rotation = QAngle_t{};
		absRotation = QAngle_t{};
		eyeAngles = QAngle_t{};
		nBoneCount = 0;
		fSimTime = 0.f;
	}

	~BacktrackRecord()
	{
		I::MemAlloc->Free(pBones);
		pBones = nullptr;
	}

	bool IsRecordValid()
	{
		float sv_maxunlag = CONVAR::sv_maxunlag->value.fl;
		sv_maxunlag = fminf(sv_maxunlag, 0.2f); // In cs2, maxunlag is clamped to 0.2

		switch (C_GET(int, Vars.nBacktrack))
		{
		case 1:
			sv_maxunlag /= 1.2f;
			break;
		case 2:
			sv_maxunlag /= 2.f;
			break;
		}

		const auto mod{ fmodf(sv_maxunlag, 0.2f) };

		const auto maxDelta{ TIME_TO_TICKS(sv_maxunlag - mod) };

		const auto overlap{ 64.f * mod };
		auto lastValid{ TIME_TO_TICKS(I::GlobalVars->flCurrentTime) - maxDelta };
		if (overlap < 1.f - 0.01f)
		{
			if (overlap <= 0.01f)
				lastValid++;
		}

		lastValid--;

		return lastValid < TIME_TO_TICKS(this->fSimTime);
	}
};

class BackupState
{
public:
	Matrix2x4_t* pBones;
	int nBoneCount;
	CNetworkOriginCellCoordQuantizedVector Origin;
	Vector_t absOrigin;
	QAngle_t Rotation;
	QAngle_t absRotation;
	QAngle_t eyeAngles;
	float fSimTime;

	BackupState()
	{
		pBones = nullptr;
		nBoneCount = 0;
		Origin = CNetworkOriginCellCoordQuantizedVector{};
		absOrigin = Vector_t{};
		Rotation = QAngle_t{};
		absRotation = QAngle_t{};
		eyeAngles = QAngle_t{};
		fSimTime = 0.f;
	}

	~BackupState()
	{
		I::MemAlloc->Free(pBones);
		pBones = nullptr;
	}
};

class C_CSPlayerPawn;

namespace F::LAGCOMP
{
	void Create(C_CSPlayerPawn* pawn);
	void Apply(C_CSPlayerPawn* pawn, size_t pos);
	void Restore(C_CSPlayerPawn* pawn);
	void Destroy(C_CSPlayerPawn* pawn);
}
