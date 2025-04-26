#pragma once

// used: matResult
#include "matrix.h"
// used: quaternion
#include "quaternion.h"

class CTransform
{
public:
	VectorAligned_t vecPosition;
	QuaternionAligned_t quatOrientation;

	Matrix3x4_t ToMatrix3x4() const
	{
		Matrix3x4_t matrix{};

		matrix[0][0] = 1.0f - 2.0f * quatOrientation.y * quatOrientation.y - 2.0f * quatOrientation.z * quatOrientation.z;
		matrix[1][0] = 2.0f * quatOrientation.x * quatOrientation.y + 2.0f * quatOrientation.w * quatOrientation.z;
		matrix[2][0] = 2.0f * quatOrientation.x * quatOrientation.z - 2.0f * quatOrientation.w * quatOrientation.y;

		matrix[0][1] = 2.0f * quatOrientation.x * quatOrientation.y - 2.0f * quatOrientation.w * quatOrientation.z;
		matrix[1][1] = 1.0f - 2.0f * quatOrientation.x * quatOrientation.x - 2.0f * quatOrientation.z * quatOrientation.z;
		matrix[2][1] = 2.0f * quatOrientation.y * quatOrientation.z + 2.0f * quatOrientation.w * quatOrientation.x;

		matrix[0][2] = 2.0f * quatOrientation.x * quatOrientation.z + 2.0f * quatOrientation.w * quatOrientation.y;
		matrix[1][2] = 2.0f * quatOrientation.y * quatOrientation.z - 2.0f * quatOrientation.w * quatOrientation.x;
		matrix[2][2] = 1.0f - 2.0f * quatOrientation.x * quatOrientation.x - 2.0f * quatOrientation.y * quatOrientation.y;

		matrix[0][3] = vecPosition.x;
		matrix[1][3] = vecPosition.y;
		matrix[2][3] = vecPosition.z;

		return matrix;
	}

	Matrix3x4_t ToMatrix3x4(const Vector_t& abs_origin) const
	{
		Matrix3x4_t matrix{};

		matrix[0][0] = 1.0f - 2.0f * quatOrientation.y * quatOrientation.y - 2.0f * quatOrientation.z * quatOrientation.z;
		matrix[1][0] = 2.0f * quatOrientation.x * quatOrientation.y + 2.0f * quatOrientation.w * quatOrientation.z;
		matrix[2][0] = 2.0f * quatOrientation.x * quatOrientation.z - 2.0f * quatOrientation.w * quatOrientation.y;

		matrix[0][1] = 2.0f * quatOrientation.x * quatOrientation.y - 2.0f * quatOrientation.w * quatOrientation.z;
		matrix[1][1] = 1.0f - 2.0f * quatOrientation.x * quatOrientation.x - 2.0f * quatOrientation.z * quatOrientation.z;
		matrix[2][1] = 2.0f * quatOrientation.y * quatOrientation.z + 2.0f * quatOrientation.w * quatOrientation.x;

		matrix[0][2] = 2.0f * quatOrientation.x * quatOrientation.z + 2.0f * quatOrientation.w * quatOrientation.y;
		matrix[1][2] = 2.0f * quatOrientation.y * quatOrientation.z - 2.0f * quatOrientation.w * quatOrientation.x;
		matrix[2][2] = 1.0f - 2.0f * quatOrientation.x * quatOrientation.x - 2.0f * quatOrientation.y * quatOrientation.y;

		matrix[0][3] = abs_origin.x;
		matrix[1][3] = abs_origin.y;
		matrix[2][3] = abs_origin.z;

		return matrix;
	}
};

static_assert(alignof(CTransform) == 16);
