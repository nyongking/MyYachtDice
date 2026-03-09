#pragma once
struct Quat
{
	float x, y, z, w;

	Quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	Quat(const Quat& rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {}

	Quat operator*(const Quat& rhs) const;
	Quat operator+(const Quat& rhs) const;
	Quat operator*(float scalar) const;

	Quat& operator+=(const Quat& rhs);
	Quat& operator*=(float scalar);
};

