#pragma once

struct Vec3
{
	float x, y, z;
	
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vec3(const Vec3& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}

	Vec3 operator+(const Vec3& rhs) const;
	Vec3 operator-(const Vec3& rhs) const;
	Vec3 operator*(float scalar) const;
	Vec3 operator/(float scalar) const;

	Vec3& operator+=(const Vec3& rhs);
	Vec3& operator-=(const Vec3& rhs);
	Vec3& operator*=(float scalar);
	Vec3& operator/=(float scalar);

	Vec3 operator-() const;
};

