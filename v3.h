#pragma once

#include <ostream>
#include <istream>

using namespace std;

class V3 {
public:
	float xyz[3];
	V3(float x, float y, float z);
	V3();
	V3 operator+(V3 v1);
	V3 operator-(V3 v1);
	float& operator[](int i);
	// cerr << "value of x: " << x << endl;
	friend ostream& operator<<(ostream &ostr, V3 v);
	friend istream& operator>>(istream& istr, V3& v); //input stream operator
	float operator*(V3 v1);
	float Length();
	V3 operator/(float scf);
	V3 operator*(float scf);
	V3 Normalized();
	V3 operator^(V3 v1);
	void rotateAxis(V3 Oa, V3 a, float theta);
	void rotateDirection(V3 a, float theta);
};