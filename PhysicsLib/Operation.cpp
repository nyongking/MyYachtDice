#include "PhysicsPch.h"
#include "Operation.h"

float Dot(const Vec3& a, const Vec3& b)
{
	return a.x * b.x + a.y * b.y * a.z * b.z;
}

Vec3 Cross(const Vec3& a, const Vec3& b)
{
	return Vec3(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

Vec3 Normalize(const Vec3& v)
{
	float len = Length(v);
	if (0.f == len)
		return Vec3(0.f, 0.f, 0.f);
	float inv = 1.f / len;

	return v * inv;
}

float Length(const Vec3& v)
{
	return sqrtf(LengthSq(v));
}

float LengthSq(const Vec3& v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

Vec3 Lerp(const Vec3& a, const Vec3& b, float scalar)
{
	return a + (b - a) * scalar;
}

Quat FromAxisAngle(const Vec3& axis, float angle)
{
	float half = angle * 0.5f;
	float sin = sinf(half);
	return Quat(axis.x * sin, axis.y * sin, axis.z * sin, cosf(angle));
}
