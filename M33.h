#pragma once

#include "v3.h"
#include <ostream>
#include <istream>

class M33 {
public:
	V3 rows[3];
	M33(V3 row0, V3 row1, V3 row2);
	M33(float m00, float m01, float m02,
		float m10, float m11, float m12,
		float m20, float m21, float m22);
	M33() {};

	static M33 Identity();
	static M33 FromColumns(V3 a, V3 b, V3 c);

	V3& operator[](int i);
	V3 GetColumn(int i);
	void SetColumn(int i, V3 v);

	V3 operator*(V3 v);
	M33 operator*(M33 m);
	M33 operator*(float scf);

	M33 Inverted();
	M33 identity(); // legacy: modifies self

	void rotationX(float theta);
	void rotationY(float theta);
	void rotationZ(float theta);

	static M33 RotationX(float degrees);
	static M33 RotationY(float degrees);
	static M33 RotationZ(float degrees);

	M33 Transposed();
	void TransposeInPlace();

	V3 ColumnSums();

	friend ostream& operator<<(ostream& os, M33& m);
	friend istream& operator>>(istream& is, M33& m);
};
