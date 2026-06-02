#pragma once

#include <ostream>
#include <istream>

using namespace std;

class V3 {
public:
	float xyz[3];
	V3(float x, float y, float z);
	V3();

	// named element access
	float& x() { return xyz[0]; }
	float& y() { return xyz[1]; }
	float& z() { return xyz[2]; }
	const float& x() const { return xyz[0]; }
	const float& y() const { return xyz[1]; }
	const float& z() const { return xyz[2]; }

	V3 operator+(V3 v1);
	V3 operator-(V3 v1);
	V3 operator-() const; // unary minus
	float& operator[](int i);
	const float& operator[](int i) const;

	// compound operators
	V3& operator+=(V3 v);
	V3& operator-=(V3 v);
	V3& operator/=(float scf);

	friend ostream& operator<<(ostream &ostr, V3 v);
	friend istream& operator>>(istream& istr, V3& v);

	float operator*(V3 v1); // dot product
	float Dot(V3 v) const;
	V3 Cross(V3 v) const;

	float Length();
	float SquareLength() const;
	float SquareDistanceTo(V3 other) const;
	float DistanceTo(V3 other);
	V3 DirectionTo(V3 other);

	V3 operator/(float scf);
	V3 operator*(float scf);
	V3 Normalized();

	V3 operator^(V3 v1); // cross product

	// rotation
	void rotateAxis(V3 Oa, V3 a, float theta);
	void rotateDirection(V3 a, float theta);
	V3 RotateThisPointAboutArbitraryAxis(V3 aO, V3 ad, float theta);
	V3 RotateThisVectorAboutArbitraryAxis(V3 ad, float theta);

	// interpolation
	V3 Interpolate(V3 other, float t) const;

	// lighting
	V3 Light(V3 normal, V3 lightDirection, float ka);
	V3 Reflect(V3 L);

	// color conversion
	void SetColor(unsigned int color);
	unsigned int GetColor();

	// comparison
	bool operator==(V3 v) const;
};
