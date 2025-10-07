#include "M33.h"
#include <cmath>


M33::M33(V3 row0, V3 row1, V3 row2) {

	rows[0] = row0;
	rows[1] = row1;
	rows[2] = row2;

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

	V3 ret;
	M33 &m = *this;
	ret[0] = m[0][i];
	ret[1] = m[1][i];
	ret[2] = m[2][i];
	return ret;

}


V3 M33::operator*(V3 v) {
	V3 ret;
	M33 &m = *this;
	ret[0] = m[0] * v;
	ret[1] = m[1] * v;
	ret[2] = m[2] * v;
	return ret;
}


M33 M33::Inverted() {

	M33 ret;
	V3 a = GetColumn(0), b = GetColumn(1), c = GetColumn(2);
	V3 _a = b ^ c; _a = _a / (a * _a);
	V3 _b = c ^ a; _b = _b / (b * _b);
	V3 _c = a ^ b; _c = _c / (c * _c);
	ret[0] = _a;
	ret[1] = _b;
	ret[2] = _c;

	return ret;

}

namespace {
	static constexpr float deg2rad = 3.14159265358979323846f / 180.0f;
}

void M33::rotationX(float theta) { //rotation about X axis
	float rad = theta * deg2rad;
	float c = cos(rad);
	float s = sin(rad);

	rows[0] = V3(1.0f, 0.0f, 0.0f);
	rows[1] = V3(0.0f, c, -s);
	rows[2] = V3(0.0f, s, c);
}

void M33::rotationY(float theta) { //rotation about Y axis
	float rad = theta * deg2rad;
	float c = cos(rad);
	float s = sin(rad);

	rows[0] = V3(c, 0.f, s);
	rows[1] = V3(0.0f, 1.0f, 0.0f);
	rows[2] = V3(-s, 0.0f, c);
}

void M33::rotationZ(float theta) { //rotation about Z axis
	float rad = theta * deg2rad;
	float c = cos(rad);
	float s = sin(rad);

	rows[0] = V3(c, -s, 0);
	rows[1] = V3(s, c, 0);
	rows[2] = V3(0, 0, 1);
}

M33 M33::Transposed() { //matrix transposition
	M33 ret;
	V3 a = GetColumn(0), b = GetColumn(1), c = GetColumn(2);
	ret[0] = a;
	ret[1] = b;
	ret[2] = c;

	return ret;
}

M33 M33::operator* (M33 m) { //matrix multiplication
	M33 ret;
	M33& m0 = *this;
	V3 a = m.GetColumn(0), b = m.GetColumn(1), c = m.GetColumn(2);
	ret[0] = V3(m0[0] * a, m0[0] * b, m0[0] * c);
	ret[1] = V3(m0[1] * a, m0[1] * b, m0[1] * c);
	ret[2] = V3(m0[2] * a, m0[2] * b, m0[2] * c);

	return ret;
}

ostream& operator<<(ostream& os, M33& m) {
	os << m.rows[0] << '\n' << m.rows[1] << '\n' << m.rows[2];
	return os;
}

istream& operator>>(istream& is, M33& m) {
	is >> m.rows[0] >> m.rows[1] >> m.rows[2];
	return is;
}



