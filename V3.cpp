#include "v3.h"
#include <cmath>
#include <iostream>

V3::V3() { // default constructor
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
	V3& v0 = (*this);
	for (int i = 0; i < 3; i++) {
		ret[i] = v0[i] + v1[i];
	}

	return ret;

}

V3 V3::operator-(V3 v1) {

	V3 ret;
	V3& v0 = (*this);
	for (int i = 0; i < 3; i++) {
		ret[i] = v0[i] - v1[i];
	}

	return ret;

}

ostream& operator<<(ostream& ostr, V3 v) {

	return ostr << v[0] << " " << v[1] << " " << v[2];

}

istream& operator>>(istream& istr, V3& v) { //input stream operator
	return istr >> v[0] >> v[1] >> v[2];
}

float& V3::operator[](int i) {

	return xyz[i];

}

float V3::operator*(V3 v1) {

	V3& v0 = *this;

	float ret = v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];

	return ret;


}

float V3::Length() {

	V3& v0 = *this;
	float ret = sqrtf(v0 * v0);
	return ret;

}

V3 V3::operator/(float scf) {

	V3 ret;
	V3& v = *this;
	ret[0] = v[0] / scf;
	ret[1] = v[1] / scf;
	ret[2] = v[2] / scf;
	return ret;

}

V3 V3::operator*(float scf) {

	V3 ret;
	V3& v = *this;
	ret[0] = v[0] * scf;
	ret[1] = v[1] * scf;
	ret[2] = v[2] * scf;
	return ret;

}


V3 V3::Normalized() {

	V3& v = *this;
	return v / v.Length();

}

V3 V3::operator^(V3 v2) {

	V3& v1 = *this;
	V3 ret(v1[1] * v2[2] - v1[2] * v2[1],
		v1[2] * v2[0] - v1[0] * v2[2],
		v1[0] * v2[1] - v1[1] * v2[0]);
	return ret;
}

static constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

float magnitude(V3 v) {
	return sqrtf(v * v);
}

void V3::rotateAxis(V3 Oa, V3 a, float theta) {
	V3 aunit = a / magnitude(a);
	V3 pRel = (*this) - Oa;
	float rad = theta * DEG_TO_RAD;

	V3 pRelRot = pRel * cos(rad) + (aunit ^ pRel) * sin(rad) + aunit * (aunit * pRel) * (1 - cos(rad));

	V3 pRot = pRelRot + Oa;
	(*this) = pRot;
}

void V3::rotateDirection(V3 a, float theta) {
	V3 Oa(0.0f, 0.0f, 0.0f);
	this->rotateAxis(Oa, a, theta);
}
