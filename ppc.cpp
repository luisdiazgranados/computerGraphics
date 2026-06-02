#include "ppc.h"
#include "framebuffer.h"

#include <cfloat>
#include <iostream>
#include <fstream>
#include <sstream>

PPC::PPC(float hfov, int _w, int _h) : hfov(hfov) {
	w = _w;
	h = _h;
	C = V3(0.0f, 0.0f, 0.0f);
	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	float hfovr = hfov * 3.1415926f / 180.0f;
	c = V3(-(float)w / 2.0f, (float)h / 2.0f,
		-(float)w / (2.0f * tan(hfovr / 2.0f)));
	Update();
}

void PPC::Update() {
	M33 M;
	M.SetColumn(0, a);
	M.SetColumn(1, b);
	M.SetColumn(2, c);
	MInv = M.Inverted();
}

int PPC::Project(V3 P, V3& PP) {
	V3 q = MInv * (P - C);

	if (q[2] <= 0.0f) {
		PP = V3(FLT_MAX, FLT_MAX, FLT_MAX);
		return 0;
	}

	PP[0] = q[0] / q[2];
	PP[1] = q[1] / q[2];
	PP[2] = 1.0f / q[2];

	return 1;
}

void PPC::Translate(V3 tv) {
	C = C + tv;
}

void PPC::TranslateGlobal(V3 tv) {
	C = C + tv;
}

void PPC::TranslateLocal(V3 tv) {
	C = C + a.Normalized() * tv[0];
	C = C - b.Normalized() * tv[1];
	C = C - (a ^ b).Normalized() * tv[2];
}

V3 PPC::GetVD() {
	return (a ^ b).Normalized();
}

float PPC::GetF() {
	return GetVD() * c;
}

void PPC::Visualize(PPC *visPPC, FrameBuffer *fb, float visF) {
	float f = GetF();
	float scf = visF / f;

	V3 topLeft = C + c * scf;
	V3 topRight = topLeft + (a * (float)w) * scf;
	V3 bottomRight = topRight + (b * (float)h) * scf;
	V3 bottomLeft = bottomRight - (a * (float)w) * scf;

	fb->Draw3DSegment(0xFF000000, visPPC, topLeft, topRight);
	fb->Draw3DSegment(0xFF000000, visPPC, topRight, bottomRight);
	fb->Draw3DSegment(0xFF000000, visPPC, bottomRight, bottomLeft);
	fb->Draw3DSegment(0xFF000000, visPPC, bottomLeft, topLeft);
	fb->Draw3DSegment(0xFF000000, visPPC, C, topLeft);
	fb->DrawPoint3D(C, visPPC, 5, 0xFF0000FF);
}

void PPC::Pan(float theta) {
	V3 ad = (b * -1.0f).Normalized();
	a = a.RotateThisVectorAboutArbitraryAxis(ad, theta);
	c = c.RotateThisVectorAboutArbitraryAxis(ad, theta);
	Update();
}

void PPC::Tilt(float theta) {
	RotateAroundDirection(a.Normalized(), theta);
}

void PPC::Roll(float theta) {
	RotateAroundDirection(GetVD(), theta);
}

void PPC::RotateAroundDirection(V3 direction, float degrees) {
	a = a.RotateThisVectorAboutArbitraryAxis(direction, degrees);
	b = b.RotateThisVectorAboutArbitraryAxis(direction, degrees);
	c = c.RotateThisVectorAboutArbitraryAxis(direction, degrees);
	Update();
}

void PPC::Zoom(float factor) {
	V3 vd = GetVD();
	float PPu = -(c * a.Normalized()) / a.Length();
	float PPv = -(c * b.Normalized()) / b.Length();
	c = a * (-PPu) - b * PPv + vd * (vd * c) * factor;
	Update();
}

void PPC::Pose(V3 newC, V3 LAP, V3 upv) {
	V3 newa, newb, newc;
	V3 newvd = (LAP - newC).Normalized();
	newa = (newvd ^ upv).Normalized();
	newb = (newvd ^ newa).Normalized();
	float f = GetF();
	newc = newvd * f - newa * ((float)w / 2.0f) - newb * ((float)h / 2.0f);
	a = newa;
	b = newb;
	c = newc;
	C = newC;
	Update();
}

V3 PPC::Unproject(int u, int v, float _1w) {
	return C + (a * (.5f + (float)u) + b * (.5f + (float)v) + c) * (1.0f / _1w);
}

PPC PPC::Interpolate(PPC other, float t) {
	PPC out(hfov, w, h);
	out.C = C + (other.C - C) * t;
	out.a = (a + (other.a - a) * t).Normalized();

	V3 vd0 = GetVD();
	V3 vd1 = other.GetVD();
	V3 vdi = (vd0 + (vd1 - vd0) * t).Normalized();

	float PPu = -(c * a.Normalized()) / a.Length();
	float PPv = -(c * b.Normalized()) / b.Length();

	out.b = (vdi ^ out.a).Normalized();
	out.c = out.a * (-PPu) - out.b * PPv + vdi * GetF();

	out.Update();
	return out;
}

void PPC::SaveToFile(const char* path) {
	ofstream o(path);
	if (o.good()) o << *this;
	else cerr << "unable to save camera to " << path << endl;
}

void PPC::LoadFromFile(const char* path) {
	ifstream i(path);
	if (i.good()) i >> *this;
	else cerr << "unable to load camera from " << path << endl;
	Update();
}

void PPC::SetHWInstrinsics() {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float nearz = 0.1f;
	float farz = 1000.0f;
	float f = GetF();
	float scalef = nearz / f;
	float wf = a.Length() * (float)w * scalef;
	float hf = b.Length() * (float)h * scalef;
	glFrustum(-wf / 2.0f, wf / 2.0f, -hf / 2.0f, hf / 2.0f, nearz, farz);
	glMatrixMode(GL_MODELVIEW);
}

void PPC::SetHWExtrinsics() {
	V3 lap = C + GetVD();
	glLoadIdentity();
	gluLookAt(C[0], C[1], C[2], lap[0], lap[1], lap[2], -b[0], -b[1], -b[2]);
}

V3 PPC::GetRay(int u, int v) {
	return (c + a * (.5f + (float)u) + b * (.5f + (float)v)).Normalized();
}

ostream& operator<<(ostream& os, PPC& ppc) {
	return os << ppc.a << ' ' << ppc.b << ' ' << ppc.c << ' '
		<< ppc.C << ' ' << ppc.w << ' ' << ppc.h;
}

istream& operator>>(istream& is, PPC& ppc) {
	is >> ppc.a >> ppc.b >> ppc.c >> ppc.C >> ppc.w >> ppc.h;
	ppc.Update();
	return is;
}
