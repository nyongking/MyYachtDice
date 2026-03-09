#pragma once

#include "Vec3.h"
#include "Quat.h"

// Vec3 ---------------------------------------------------------

float	Dot(const Vec3& a, const Vec3& b);
Vec3	Cross(const Vec3& a, const Vec3& b);
Vec3	Normalize(const Vec3& v);
float	Length(const Vec3& v);
float	LengthSq(const Vec3& v);
Vec3	Lerp(const Vec3& a, const Vec3& b, float scalar);

// Quat ---------------------------------------------------------

Quat FromAxisAngle(const Vec3& axis, float angle);