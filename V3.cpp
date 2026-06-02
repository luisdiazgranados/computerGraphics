#include "v3.h"
#include <cmath>
#include <iostream>

V3::V3() {
	xyz[0] = 0.0f;
	xyz[1] = 0.0f;
	xyz[2] = 0.0f;
}

V3::V3(float x, float y, float z) {
	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;
}

V3 V3::operator+(V3 v1) {
	V3 ret;
	for (int i = 0; i < 3; i++) {
		ret[i] = xyz[i] + v1[i];
	}
	return ret;
}

V3 V3::operator-(V3 v1) {
	V3 ret;
	for (int i = 0; i < 3; i++) {
		ret[i] = xyz[i] - v1[i];
	}
	return ret;
}

V3 V3::operator-() const {
	return V3(-xyz[0], -xyz[1], -xyz[2]);
}

float& V3::operator[](int i) {
	return xyz[i];
}

const float& V3::operator[](int i) const {
	return xyz[i];
}

V3& V3::operator+=(V3 v) {
	for (int i = 0; i < 3; i++)
		xyz[i] += v[i];
	return *this;
}

V3& V3::operator-=(V3 v) {
	for (int i = 0; i < 3; i++)
		xyz[i] -= v[i];
	return *this;
}

V3& V3::operator/=(float scf) {
	for (int i = 0; i < 3; i++)
		xyz[i] /= scf;
	return *this;
}

ostream& operator<<(ostream& ostr, V3 v) {
	return ostr << v[0] << " " << v[1] << " " << v[2];
}

istream& operator>>(istream& istr, V3& v) {
	return istr >> v[0] >> v[1] >> v[2];
}

float V3::operator*(V3 v1) {
	return xyz[0] * v1[0] + xyz[1] * v1[1] + xyz[2] * v1[2];
}

float V3::Dot(V3 v) const {
	return xyz[0] * v[0] + xyz[1] * v[1] + xyz[2] * v[2];
}

V3 V3::Cross(V3 v) const {
	return V3(
		xyz[1] * v[2] - xyz[2] * v[1],
		xyz[2] * v[0] - xyz[0] * v[2],
		xyz[0] * v[1] - xyz[1] * v[0]);
}

float V3::Length() {
	return sqrtf(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);
}

float V3::SquareLength() const {
	return xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2];
}

float V3::SquareDistanceTo(V3 other) const {
	float dx = xyz[0] - other[0];
	float dy = xyz[1] - other[1];
	float dz = xyz[2] - other[2];
	return dx * dx + dy * dy + dz * dz;
}

float V3::DistanceTo(V3 other) {
	return (*this - other).Length();
}

V3 V3::DirectionTo(V3 other) {
	return (other - *this).Normalized();
}

V3 V3::operator/(float scf) {
	V3 ret;
	ret[0] = xyz[0] / scf;
	ret[1] = xyz[1] / scf;
	ret[2] = xyz[2] / scf;
	return ret;
}

V3 V3::operator*(float scf) {
	V3 ret;
	ret[0] = xyz[0] * scf;
	ret[1] = xyz[1] * scf;
	ret[2] = xyz[2] * scf;
	return ret;
}

V3 V3::Normalized() {
	return *this / Length();
}

V3 V3::operator^(V3 v2) {
	return V3(xyz[1] * v2[2] - xyz[2] * v2[1],
		xyz[2] * v2[0] - xyz[0] * v2[2],
		xyz[0] * v2[1] - xyz[1] * v2[0]);
}

static constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

void V3::rotateAxis(V3 Oa, V3 a, float theta) {
	V3 aunit = a.Normalized();
	V3 pRel = (*this) - Oa;
	float rad = theta * DEG_TO_RAD;

	V3 pRelRot = pRel * cos(rad) + (aunit ^ pRel) * sin(rad) + aunit * (aunit * pRel) * (1 - cos(rad));

	*this = pRelRot + Oa;
}

void V3::rotateDirection(V3 a, float theta) {
	rotateAxis(V3(0.0f, 0.0f, 0.0f), a, theta);
}

V3 V3::RotateThisPointAboutArbitraryAxis(V3 aO, V3 ad, float theta) {
	V3 ret = *this;
	ret.rotateAxis(aO, ad, theta);
	return ret;
}

V3 V3::RotateThisVectorAboutArbitraryAxis(V3 ad, float theta) {
	return RotateThisPointAboutArbitraryAxis(V3(0.0f, 0.0f, 0.0f), ad, theta);
}

V3 V3::Interpolate(V3 other, float t) const {
	return V3(
		xyz[0] + (other[0] - xyz[0]) * t,
		xyz[1] + (other[1] - xyz[1]) * t,
		xyz[2] + (other[2] - xyz[2]) * t);
}

V3 V3::Light(V3 normal, V3 lightDirection, float ka) {
	float kd = normal * lightDirection;
	if (kd < 0.0f) kd = 0.0f;
	return *this * (ka + (1.0f - ka) * kd);
}

V3 V3::Reflect(V3 L) {
	V3 n = *this;
	V3 Ln = n * n.Dot(L);
	V3 Lt = L - Ln;
	return Ln - Lt;
}

void V3::SetColor(unsigned int color) {
	xyz[0] = (float)((color >> 0) & 0xFF) / 255.0f;
	xyz[1] = (float)((color >> 8) & 0xFF) / 255.0f;
	xyz[2] = (float)((color >> 16) & 0xFF) / 255.0f;
}

unsigned int V3::GetColor() {
	unsigned int r = (unsigned int)(xyz[0] * 255.0f);
	unsigned int g = (unsigned int)(xyz[1] * 255.0f);
	unsigned int b = (unsigned int)(xyz[2] * 255.0f);
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	return (0xFF << 24) | (b << 16) | (g << 8) | r;
}

bool V3::operator==(V3 v) const {
	return xyz[0] == v[0] && xyz[1] == v[1] && xyz[2] == v[2];
}
