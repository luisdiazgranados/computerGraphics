#include "TM.h"

#include <fstream>
#include <iostream>

using namespace std;

void TM::DrawPoints(unsigned int color, int psize, PPC *ppc, 
	FrameBuffer *fb) {

	for (int vi = 0; vi < vertsN; vi++) {
		fb->DrawPoint3D(verts[vi], ppc, psize, color);
	}

}

void TM::Translate(V3 tv) {

	for (int vi = 0; vi < vertsN; vi++) {
		verts[vi] = verts[vi] + tv;
	}

}


V3 TM::GetCenter() {

	V3 ret(0.0f, 0.0f, 0.0f);
	for (int vi = 0; vi < vertsN; vi++) {
		ret = ret + verts[vi];
	}
	return ret / (float) vertsN;

}

void TM::RotateAboutArbitraryAxis(V3 aO, V3 ad, float theta) {

	for (int vi = 0; vi < vertsN; vi++) {
		verts[vi] = verts[vi].RotateThisPointAboutArbitraryAxis(aO, ad, theta);
	}

}


void TM::Position(V3 newCenter) {

	V3 oldCenter = GetCenter();
	Translate(newCenter - oldCenter);

}


// loading triangle mesh from a binary file, i.e., a .bin file from geometry folder
void TM::LoadBin(char *fname) {

	ifstream ifs(fname, ios::binary);
	if (ifs.fail()) {
		cerr << "INFO: cannot open file: " << fname << endl;
		return;
	}

	ifs.read((char*)&vertsN, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	if (verts)
		delete verts;
	verts = new V3[vertsN];

	ifs.read(&yn, 1); // cols 3 floats
	if (colors)
		delete colors;
	colors = 0;
	if (yn == 'y') {
		colors = new V3[vertsN];
	}

	ifs.read(&yn, 1); // normals 3 floats
	if (normals)
		delete []normals;
	normals = 0;
	if (yn == 'y') {
		normals = new V3[vertsN];
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats
	float *tcs = 0; // don't have texture coordinates for now
	if (tcs)
		delete []tcs;
	tcs = 0;
	if (yn == 'y') {
		tcs = new float[vertsN * 2];
	}


	ifs.read((char*)verts, vertsN * 3 * sizeof(float)); // load verts

	if (colors) {
		ifs.read((char*)colors, vertsN * 3 * sizeof(float)); // load colors
	}

	if (normals)
		ifs.read((char*)normals, vertsN * 3 * sizeof(float)); // load normals

	if (tcs)
		ifs.read((char*)tcs, vertsN * 2 * sizeof(float)); // load texture coordinates

	ifs.read((char*)&trisN, sizeof(int));
	if (tris)
		delete tris;
	tris = new unsigned int[trisN * 3];
	ifs.read((char*)tris, trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((colors) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

}



void TM::DrawWireFrame(unsigned int color, PPC *ppc, FrameBuffer *fb) {

	for (int tri = 0; tri < trisN; tri++) {
		V3 Vs[3], Cs[3];
		Vs[0] = verts[tris[tri * 3 + 0]];
		Vs[1] = verts[tris[tri * 3 + 1]];
		Vs[2] = verts[tris[tri * 3 + 2]];
		if (colors) {
			Cs[0] = colors[tris[tri * 3 + 0]];
			Cs[1] = colors[tris[tri * 3 + 1]];
			Cs[2] = colors[tris[tri * 3 + 2]];
		}
		for (int ei = 0; ei < 3; ei++) {
			if (colors)
				fb->Draw3DSegment(Cs[ei], Cs[(ei + 1) % 3], ppc, 
					Vs[ei], Vs[(ei + 1) % 3]);
			else
				fb->Draw3DSegment(color, ppc, Vs[ei], Vs[(ei + 1) % 3]);
		}
	}

}

void TM::VisualizeNormals(float nl, PPC *ppc, FrameBuffer *fb) {

	if (!normals)
		return;

	for (int vi = 0; vi < vertsN; vi++) {
		fb->Draw3DSegment(colors[vi], V3(1.0f, 0.0f, 0.0f), ppc,
			verts[vi], verts[vi] + normals[vi].Normalized() * nl);
	}

}

void TM::LightD(V3 ld, float ka) {

	for (int vi = 0; vi < vertsN; vi++) {
		colors[vi] = V3(1.0f, 0.0f, 0.0f);
		colors[vi] = colors[vi].Light(normals[vi], ld, ka);
	}

}

void TM::LightP(V3 L, float ka) {

	for (int vi = 0; vi < vertsN; vi++) {
		colors[vi] = V3(1.0f, 0.0f, 0.0f);
		V3 ld = (L - verts[vi]).Normalized();
		colors[vi] = colors[vi].Light(normals[vi], ld, ka);
	}

}