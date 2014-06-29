
#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

struct vec3
{
	float x, y, z;

	vec3();
	vec3(float x, float y, float z);

	vec3 operator -() const;
	vec3 operator +(float s) const;
	vec3 operator -(float s) const;
	vec3 operator *(float s) const;
	vec3 operator /(float s) const;
	vec3 operator +(const vec3& v) const;
	vec3 operator -(const vec3& v) const;
	const vec3& operator +=(const vec3& v);
	const vec3& operator -=(const vec3& v);
	bool operator ==(const vec3& v) const;
	bool operator !=(const vec3& v) const;
};

float dot(const vec3& v1, const vec3& v2);
float length(const vec3& v);
float lengthSquare(const vec3& v);
vec3 normalize(const vec3& v);
vec3 cross(const vec3& v1, const vec3& v2);

//------------------------------------------------------------------------------------

struct mat3
{
	float m[9];

	mat3();

	vec3 operator * (const vec3& v);
	mat3 operator * (const mat3& other);
};

mat3 RotationMatrix(float angle, const vec3& aroundDir);

//------------------------------------------------------------------------------------

struct mat4
{
	float m[16];

	mat4();
};