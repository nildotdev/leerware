#include "vector.h"

#include "matrix.h"
#include "qangle.h"

// used: m_rad2deg
#include "../../utilities/math.h"

[[nodiscard]] Vector_t Vector_t::Transform(const Matrix3x4_t& matTransform) const
{
	return {
		this->DotProduct(matTransform[0]) + matTransform[0][3],
		this->DotProduct(matTransform[1]) + matTransform[1][3],
		this->DotProduct(matTransform[2]) + matTransform[2][3]
	};
}

[[nodiscard]] QAngle_t Vector_t::ToAngles() const
{
	float flPitch, flYaw;
	if (this->x == 0.0f && this->y == 0.0f)
	{
		flPitch = (this->z > 0.0f) ? 270.f : 90.f;
		flYaw = 0.0f;
	}
	else
	{
		flPitch = M_RAD2DEG(std::atan2f(-this->z, this->Length2D()));

		if (flPitch < 0.f)
			flPitch += 360.f;

		flYaw = M_RAD2DEG(std::atan2f(this->y, this->x));

		if (flYaw < 0.f)
			flYaw += 360.f;
	}

	return { flPitch, flYaw, 0.0f };
}

[[nodiscard]] Matrix3x4_t Vector_t::ToMatrix() const
{
	Vector_t vecRight = {}, vecUp = {};
	this->ToDirections(&vecRight, &vecUp);

	Matrix3x4a_t matOutput = {};
	matOutput.SetForward(*this);
	matOutput.SetLeft(-vecRight);
	matOutput.SetUp(vecUp);
	return matOutput;
}

[[nodiscard]] QAngle_t Vector4D_t::ToEulerAngles() const
{
	QAngle_t angles;
	const auto x = this->x;
	const auto y = this->y;
	const auto z = this->z;
	const auto w = this->w;

	double sinr_cosp = 2 * (w * x + y * z);
	double cosr_cosp = 1 - 2 * (x * x + y * y);
	angles[2] = std::atan2(sinr_cosp, cosr_cosp);

	double sinp = 2 * (w * y - z * x);
	if (std::abs(sinp) >= 1)
		angles[1] = std::copysign(MATH::_PI / 2, sinp);
	else
		angles[1] = std::asin(sinp);

	double siny_cosp = 2 * (w * z + x * y);
	double cosy_cosp = 1 - 2 * (y * y + z * z);
	angles[0] = std::atan2(siny_cosp, cosy_cosp);
	return QAngle_t(M_RAD2DEG(angles.y), M_RAD2DEG(angles.x), M_RAD2DEG(angles.z));
}
