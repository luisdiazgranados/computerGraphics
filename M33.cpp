#include "M33.h"
#include <cmath>
#include <algorithm>

M33::M33(V3 row0, V3 row1, V3 row2) {
	rows[0] = row0;
	rows[1] = row1;
	rows[2] = row2;
}

M33::M33(float m00, float m01, float m02,
	float m10, float m11, float m12,
	float m20, float m21, float m22) {
	rows[0] = V3(m00, m01, m02);
	rows[1] = V3(m10, m11, m12);
	rows[2] = V3(m20, m21, m22);
}

M33 M33::Identity() {
	return M33(
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f);
}

M33 M33::FromColumns(V3 a, V3 b, V3 c) {
	return M33(a, b, c).Transposed();
}

M33 M33::identity() {
	rows[0] = V3(1.0f, 0.0f, 0.0f);
	rows[1] = V3(0.0f, 1.0f, 0.0f);
	rows[2] = V3(0.0f, 0.0f, 1.0f);
	return *this;
}

V3& M33::operator[](int i) {
	return rows[i];
}

V3 M33::GetColumn(int i) {
	return V3(rows[0][i], rows[1][i], rows[2][i]);
}

void M33::SetColumn(int i, V3 v) {
	rows[0][i] = v[0];
	rows[1][i] = v[1];
	rows[2][i] = v[2];
}

V3 M33::operator*(V3 v) {
	return V3(rows[0] * v, rows[1] * v, rows[2] * v);
}

M33 M33::Inverted() {
	V3 a = GetColumn(0), b = GetColumn(1), c = GetColumn(2);
	V3 _a = b ^ c;
	float det = a * _a;
	if (det == 0.0f)
		return M33::Identity();
	_a = _a / (a * _a);
	V3 _b = c ^ a; _b = _b / (b * _b);
	V3 _c = a ^ b; _c = _c / (c * _c);
	M33 ret;
	ret[0] = _a;
	ret[1] = _b;
	ret[2] = _c;
	return ret;
}

namespace {
	static constexpr float deg2rad = 3.14159265358979323846f / 180.0f;
}

void M33::rotationX(float theta) {
	float rad = theta * deg2rad;
	float c = cos(rad), s = sin(rad);
	rows[0] = V3(1.0f, 0.0f, 0.0f);
	rows[1] = V3(0.0f, c, -s);
	rows[2] = V3(0.0f, s, c);
}

void M33::rotationY(float theta) {
	float rad = theta * deg2rad;
	float c = cos(rad), s = sin(rad);
	rows[0] = V3(c, 0.0f, s);
	rows[1] = V3(0.0f, 1.0f, 0.0f);
	rows[2] = V3(-s, 0.0f, c);
}

void M33::rotationZ(float theta) {
	float rad = theta * deg2rad;
	float c = cos(rad), s = sin(rad);
	rows[0] = V3(c, -s, 0.0f);
	rows[1] = V3(s, c, 0.0f);
	rows[2] = V3(0.0f, 0.0f, 1.0f);
}

M33 M33::RotationX(float degrees) {
	M33 ret;
	ret.rotationX(degrees);
	return ret;
}

M33 M33::RotationY(float degrees) {
	M33 ret;
	ret.rotationY(degrees);
	return ret;
}

M33 M33::RotationZ(float degrees) {
	M33 ret;
	ret.rotationZ(degrees);
	return ret;
}

M33 M33::Transposed() {
	return M33(GetColumn(0), GetColumn(1), GetColumn(2));
}

void M33::TransposeInPlace() {
	std::swap(rows[0][1], rows[1][0]);
	std::swap(rows[0][2], rows[2][0]);
	std::swap(rows[1][2], rows[2][1]);
}

M33 M33::operator*(M33 m) {
	V3 a = m.GetColumn(0), b = m.GetColumn(1), c = m.GetColumn(2);
	return M33(
		V3(rows[0] * a, rows[0] * b, rows[0] * c),
		V3(rows[1] * a, rows[1] * b, rows[1] * c),
		V3(rows[2] * a, rows[2] * b, rows[2] * c));
}

M33 M33::operator*(float scf) {
	return M33(rows[0] * scf, rows[1] * scf, rows[2] * scf);
}

V3 M33::ColumnSums() {
	return V3(
		rows[0][0] + rows[1][0] + rows[2][0],
		rows[0][1] + rows[1][1] + rows[2][1],
		rows[0][2] + rows[1][2] + rows[2][2]);
}

ostream& operator<<(ostream& os, M33& m) {
	os << m.rows[0] << '\n' << m.rows[1] << '\n' << m.rows[2];
	return os;
}

istream& operator>>(istream& is, M33& m) {
	is >> m.rows[0] >> m.rows[1] >> m.rows[2];
	return is;
}
