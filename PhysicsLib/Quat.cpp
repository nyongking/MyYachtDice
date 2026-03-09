#include "PhysicsPch.h"
#include "Quat.h"

Quat Quat::operator*(const Quat& rhs) const
{
	return Quat(w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
		w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
		w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
		w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
}

Quat Quat::operator+(const Quat& rhs) const
{
	return Quat(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

Quat Quat::operator*(float scalar) const
{
	return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
}

Quat& Quat::operator+=(const Quat& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;

	return *this;
}

Quat& Quat::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	w *= scalar;

	return *this;
}