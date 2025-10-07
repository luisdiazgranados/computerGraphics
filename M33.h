#pragma once

#include "v3.h"
#include <ostream>
#include <istream>

class M33 {
public:
	V3 rows[3];
	M33(V3 row0, V3 row1, V3 row2);
	V3& operator[](int i);
	V3 GetColumn(int i);
	V3 operator*(V3 v);
	M33 Inverted();
	M33() {}; //uninitialized matrix constructor
	M33 identity();
	void rotationX(float theta);
	void rotationY(float theta);
	void rotationZ(float theta);
	M33 Transposed();
	M33 operator*(M33 m);
	friend ostream& operator<<(ostream& os, M33& m);
	friend istream& operator>>(istream& is, M33& m);
};