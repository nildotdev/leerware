#pragma once
#include "../../core/interfaces.h"
#include "../../utilities/fnv1a.h"
#include "../../sdk/datatypes/matrix.h"
#include "../../sdk/interfaces/ienginecvar.h"

class BacktrackRecord
{
public:
	Matrix2x4_t* bones;
	int pBoneCount;
	float simTime;

	bool IsRecordValid()
	{
		float sv_maxunlag = I::Cvar->Find(FNV1A::HashConst("sv_maxunlag"))->value.fl;
		sv_maxunlag = fminf(sv_maxunlag, 0.2f); // In cs2, maxunlag is clamped to 0.2

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

		return lastValid < TIME_TO_TICKS(this->simTime);
	}
};

namespace F::LAGCOMP
{
	void Save();
	void Apply();
	void Restore();
}
