#pragma once

#include "v3.h"
#include "M33.h"
class FrameBuffer;

class PPC {
public:
	V3 a, b, c, C;
	int w, h;
	float hfov;
	M33 MInv; // cached inverse of [a b c] matrix

	PPC() {};
	PPC(float hfov, int _w, int _h);

	void Update(); // recompute MInv

	int Project(V3 P, V3& PP);
	void Translate(V3 tv);
	void TranslateGlobal(V3 tv);
	void TranslateLocal(V3 tv);

	V3 GetVD();
	float GetF();

	void Visualize(PPC *visPPC, FrameBuffer *fb, float visF);

	void Pan(float theta);
	void Tilt(float theta);
	void Roll(float theta);
	void RotateAroundDirection(V3 direction, float degrees);

	void Zoom(float factor);
	void ZoomIn() { Zoom(1.1f); }
	void ZoomOut() { Zoom(1.0f / 1.1f); }

	void Pose(V3 newC, V3 LAP, V3 upv);

	V3 Unproject(int u, int v, float _1w);

	PPC Interpolate(PPC other, float t);
	PPC InterpolateSmooth(PPC other, float t) {
		float s = t * t * (3.0f - 2.0f * t);
		return Interpolate(other, s);
	}

	void SaveToFile(const char* path);
	void LoadFromFile(const char* path);

	void SetHWInstrinsics();
	void SetHWExtrinsics();
	V3 GetRay(int u, int v);

	friend ostream& operator<<(ostream& os, PPC& ppc);
	friend istream& operator>>(istream& is, PPC& ppc);
};
