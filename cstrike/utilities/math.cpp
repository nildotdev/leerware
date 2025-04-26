#include "math.h"
#include "../sdk/datatypes/qangle.h"

//used: getexportaddr
#include "memory.h"

bool MATH::Setup()
{
	bool bSuccess = true;

	const void* hTier0Lib = MEM::GetModuleBaseHandle(TIER0_DLL);
	if (hTier0Lib == nullptr)
		return false;

	fnRandomSeed = reinterpret_cast<decltype(fnRandomSeed)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomSeed")));
	bSuccess &= (fnRandomSeed != nullptr);

	fnRandomFloat = reinterpret_cast<decltype(fnRandomFloat)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomFloat")));
	bSuccess &= (fnRandomFloat != nullptr);

	fnRandomFloatExp = reinterpret_cast<decltype(fnRandomFloatExp)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomFloatExp")));
	bSuccess &= (fnRandomFloatExp != nullptr);

	fnRandomInt = reinterpret_cast<decltype(fnRandomInt)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomInt")));
	bSuccess &= (fnRandomInt != nullptr);

	fnRandomGaussianFloat = reinterpret_cast<decltype(fnRandomGaussianFloat)>(MEM::GetExportAddress(hTier0Lib, CS_XOR("RandomGaussianFloat")));
	bSuccess &= (fnRandomGaussianFloat != nullptr);

	return bSuccess;
}

void MATH::VectorAngles(const Vector_t& forward, QAngle_t& angles, Vector_t* up)
{
	Vector_t left;
	float len, up_z, pitch, yaw, roll;

	// get 2d length.
	len = forward.Length2D();

	if (up && len > 0.001f)
	{
		pitch = M_RAD2DEG(atan2(-forward.z, len));
		yaw = M_RAD2DEG(atan2(forward.y, forward.x));

		// get left direction vector using cross product.
		left = (*up).CrossProduct(forward).Normalized();

		// calculate up_z.
		up_z = (left.y * forward.x) - (left.x * forward.y);

		// calculate roll.
		roll = M_RAD2DEG(atan2(left.z, up_z));
	}

	else
	{
		if (len > 0.f)
		{
			// calculate pitch and yaw.
			pitch = M_RAD2DEG(atan2(-forward.z, len));
			yaw = M_RAD2DEG(atan2(forward.y, forward.x));
			roll = 0.f;
		}

		else
		{
			pitch = (forward.z > 0.f) ? -90.f : 90.f;
			yaw = 0.f;
			roll = 0.f;
		}
	}

	// set out angles.
	angles = { pitch, yaw, roll };
}

void MATH::AngleVectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up)
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

QAngle_t MATH::CalculateAngles(Vector_t from, Vector_t to)
{
	// Ported from my C# external
	float yaw;
	float pitch;

	float deltaX = to.x - from.x;
	float deltaY = to.y - from.y;
	yaw = (float)(atan2(deltaY, deltaX) * 180 / MATH::_PI);

	float deltaZ = to.z - from.z;
	double distance = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
	pitch = -(float)(atan2(deltaZ, distance) * 180 / MATH::_PI);

	return QAngle_t(pitch, yaw, 0.f);
}

float MATH::CalculateFOVDistance(const QAngle_t& ang1, const QAngle_t& ang2)
{
	Vector_t vec1;
	Vector_t vec2;
	MATH::AngleVectors(ang1, &vec1);
	MATH::AngleVectors(ang2, &vec2);

	float dot = vec1.DotProduct(vec2);
	dot = MATH::Clamp(dot, -1.0f, 1.0f);

	return M_RAD2DEG(acosf(dot));
}
