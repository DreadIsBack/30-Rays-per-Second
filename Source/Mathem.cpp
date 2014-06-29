#include "Mathem.h"

vec3::vec3() :x(0.0f), y(0.0f), z(0.0f)
{
}

vec3::vec3(float x, float y, float z) : x(x), y(y), z(z)
{
}

vec3 vec3::operator -() const
{
	return vec3(-x, -y, -z);
}

vec3 vec3::operator +(float s) const
{
	return vec3(x + s, y + s, z + s);
}

vec3 vec3::operator -(float s) const
{
	return vec3(x - s, y - s, z - s);
}

vec3 vec3::operator *(float s) const
{
	return vec3(x * s, y * s, z * s);
}

vec3 vec3::operator /(float s) const
{
	return vec3(x / s, y / s, z / s);
}

vec3 vec3::operator +(const vec3& v) const
{
	return vec3(x + v.x, y + v.y, z + v.z);
}

vec3 vec3::operator -(const vec3& v) const
{
	return vec3(x - v.x, y - v.y, z - v.z);
}

const vec3& vec3::operator +=(const vec3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

const vec3& vec3::operator -=(const vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

bool vec3::operator ==(const vec3& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

bool vec3::operator !=(const vec3& v) const
{
	return !(*this == v);
}

float dot(const vec3& v1, const vec3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float length(const vec3& v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float lengthSquare(const vec3& v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

vec3 normalize(const vec3& v)
{
	return v * (1.0f / sqrt(v.x * v.x + v.y * v.y + v.z * v.z));
}

vec3 cross(const vec3& v1, const vec3& v2)
{
	return vec3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

//------------------------------------------------------------------------------------

mat3::mat3()
{
	m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f;
	m[3] = 0.0f; m[4] = 1.0f; m[5] = 0.0f;
	m[6] = 0.0f; m[7] = 0.0f; m[8] = 1.0f;
}

vec3 mat3::operator * (const vec3& v)
{
	vec3 res;

	res.x = m[0] * v.x + m[3] * v.y + m[6] * v.z;
	res.y = m[1] * v.x + m[4] * v.y + m[7] * v.z;
	res.z = m[2] * v.x + m[5] * v.y + m[8] * v.z;

	return res;
}

mat3 mat3::operator * (const mat3& other)
{
	mat3 res;
	res.m[0] = res.m[4] = res.m[8] = 0.0f; // zero matrix

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++)
				res.m[i * 3 + j] += m[i * 3 + k] * other.m[k * 3 + j];

	return res;
}

mat3 RotationMatrix(float angle, const vec3& aroundDir)
{
	mat3 R;

	angle = angle / 180.0f * (float)M_PI;

	vec3 v = normalize(aroundDir); // TODO: need?

	float c = 1.0f - cos(angle);
	float s = sin(angle);

	R.m[0] = 1.0f + c * (v.x * v.x - 1.0f);
	R.m[1] = c * v.x * v.y + v.z * s;
	R.m[2] = c * v.x * v.z - v.y * s;

	R.m[3] = c * v.x * v.y - v.z * s;
	R.m[4] = 1.0f + c * (v.y * v.y - 1.0f);
	R.m[5] = c * v.y * v.z + v.x * s;

	R.m[6] = c * v.x * v.z + v.y * s;
	R.m[7] = c * v.y * v.z - v.x * s;
	R.m[8] = 1.0f + c * (v.z * v.z - 1.0f);

	return R;
}

//------------------------------------------------------------------------------------

mat4::mat4()
{
	m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
	m[4] = 0.0f; m[5] = 1.0f; m[6] = 0.0f; m[7] = 0.0f;
	m[8] = 0.0f; m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
	m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}