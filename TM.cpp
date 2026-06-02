#include "TM.h"
#include "color.h"
#include "cubemap.h"

#include <fstream>
#include <iostream>
#include <cfloat>
#include <cmath>
#include <algorithm>

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
	if (normals) {
		for (int vi = 0; vi < vertsN; vi++) {
			normals[vi] = normals[vi].RotateThisVectorAboutArbitraryAxis(ad, theta);
		}
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

void TM::SetEEQs(M33 pvs, M33 &eeqs) {

	for (int ei = 0; ei < 3; ei++) {
		int ei1 = (ei + 1) % 3;
		float x0 = pvs[ei][0];
		float y0 = pvs[ei][1];
		float x1 = pvs[ei1][0];
		float y1 = pvs[ei1][1];
		eeqs[ei][0] = y1 - y0;
		eeqs[ei][1] = -x1 + x0;
		eeqs[ei][2] = y0*(x1 - x0) - x0*(y1 - y0);
		int ei2 = (ei + 2) % 3;
		if (eeqs[ei] * pvs[ei2] < 0.0f)
			eeqs[ei] = eeqs[ei] * -1.0f;
	}

}

void TM::RenderFilled(PPC *ppc, FrameBuffer *fb) {

	if (!projverts)
		projverts = new V3[vertsN];

	for (int vi = 0; vi < vertsN; vi++) {
		ppc->Project(verts[vi], projverts[vi]);
	}

	for (int tri = 0; tri < trisN; tri++) {
		if (projverts[tris[3 * tri + 0]][0] == FLT_MAX)
			continue;
		if (projverts[tris[3 * tri + 1]][0] == FLT_MAX)
			continue;
		if (projverts[tris[3 * tri + 2]][0] == FLT_MAX)
			continue;
		AABB aabb(projverts[tris[3 * tri + 0]]);
		aabb.AddPoint(projverts[tris[3 * tri + 1]]);
		aabb.AddPoint(projverts[tris[3 * tri + 2]]);
		int left = (int)(aabb.c0[0]+.5f);
		int right = (int)(aabb.c1[0]-.5f);
		int top = (int)(aabb.c0[1]+.5f);
		int bottom = (int)(aabb.c1[1]-.5f);
		M33 pvs;
		pvs[0] = projverts[tris[3 * tri + 0]];
		pvs[1] = projverts[tris[3 * tri + 1]];
		pvs[2] = projverts[tris[3 * tri + 2]];
		V3 pvzs = pvs.GetColumn(2);
		pvs.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		V3 zabc = pvs.Inverted()*pvzs;
		M33 colsm;
		colsm[0] = colors[tris[3 * tri + 0]];
		colsm[1] = colors[tris[3 * tri + 1]];
		colsm[2] = colors[tris[3 * tri + 2]];
		M33 rgbabc = (pvs.Inverted()*colsm).Transposed();
		M33 tcsm;
		if (tcs) {
			tcsm[0] = tcs[tris[3 * tri + 0]];
			tcsm[1] = tcs[tris[3 * tri + 1]];
			tcsm[2] = tcs[tris[3 * tri + 2]];
		}
		M33 tcsabc = (pvs.Inverted()*tcsm).Transposed();
		M33 eeqs; SetEEQs(pvs, eeqs);

		for (int v = top; v <= bottom; v++) {
			for (int u = left; u <= right; u++) {
				V3 pixcv(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 sdv = eeqs*pixcv;
				if (sdv[0] < 0.0f || sdv[1] < 0.0f || sdv[2] < 0.0f)
					continue;
				float currz = zabc*pixcv;
				if (fb->IsFarther(u, v, currz))
					continue;
				V3 currcol = rgbabc * pixcv;
				if (tex) {
					V3 currtc = tcsabc * pixcv;
					int tu = (int)(currtc[0] * (float)tex->w);
					int tv = (int)(currtc[1] * (float)tex->h);
					currcol.SetColor(tex->Get(tu, tv));
				}
				fb->SetSafe(u, v, currcol.GetColor());
				fb->SetSafeZB(u, v, currz);
			}
		}
	}

}


void TM::SetQuad(V3 *vs) {

	vertsN = 4;
	trisN = 2;
	
	//  write allocate method
	verts = new V3[vertsN];
	projverts = new V3[vertsN];
	normals = new V3[vertsN];
	colors = new V3[vertsN];
	tcs = new V3[vertsN];
	tris = new unsigned int[3 * trisN];

	for (int vi = 0; vi < vertsN; vi++)
		verts[vi] = vs[vi];
	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;
	tri++;
	tris[3 * tri + 0] = 2;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 0;

	tcs[0] = V3(0.0f, 0.0f, 1.0f);
	tcs[1] = V3(0.0f, 1.0f, 1.0f);
	tcs[2] = V3(1.0f, 1.0f, 1.0f);
	tcs[3] = V3(1.0f, 0.0f, 1.0f);

	normals[0] =
		normals[1] =
		normals[2] =
		normals[3] = ((vs[1] - vs[0]) ^ (vs[3] - vs[0])).Normalized();


}


void TM::RenderHW() {

	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (float*)verts);

	if (normals) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, 0, (float*)normals);
	}

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(3, GL_FLOAT, 0, (float*)colors);

	// enable and pass texture coordinate array
	// enables texture to be used for this TM

	glDrawElements(GL_TRIANGLES, 3*trisN, GL_UNSIGNED_INT, tris);


	// disable texture
	// disable texture coordinate array

	glDisableClientState(GL_NORMAL_ARRAY);

	glDisableClientState(GL_COLOR_ARRAY);

	glDisableClientState(GL_VERTEX_ARRAY);

}


AABB TM::GetAABB() {

	AABB ret(verts[0]);
	for (int vi = 0; vi < vertsN; vi++) {
		ret.AddPoint(verts[vi]);
	}
	return ret;

}

int TM::IntersectWithRay(V3 O, V3 ray, V3 &color, float &t, V3 &rO, V3 &rray) {

	t = FLT_MAX;
	for (int tri = 0; tri < trisN; tri++) {
		M33 M;
		M.SetColumn(0, verts[tris[3 * tri + 0]]);
		M.SetColumn(1, verts[tris[3 * tri + 1]]);
		M.SetColumn(2, verts[tris[3 * tri + 2]]);
		M33 M1 = M.Inverted();
		V3 Q0 = M1*O;
		V3 Q1 = M1*ray;
		float ct = (1.0f-(Q0[0] + Q0[1] + Q0[2])) / (Q1[0] + Q1[1] + Q1[2]);
		if (ct < 0.0f)
			continue;
		V3 abc = Q0+Q1*ct; // barycentric coordinates
		if (abc[0] < 0.0f || abc[1] < 0.0f || abc[2] < 0.0f)
			continue;

		if (ct < t) {
			if (!tex) {
				M33 MC;
				MC.SetColumn(0, colors[tris[3 * tri + 0]]);
				MC.SetColumn(1, colors[tris[3 * tri + 1]]);
				MC.SetColumn(2, colors[tris[3 * tri + 2]]);
				color = MC*abc;
			}
			else {
				M33 MT;
				MT.SetColumn(0, tcs[tris[3 * tri + 0]]);
				MT.SetColumn(1, tcs[tris[3 * tri + 1]]);
				MT.SetColumn(2, tcs[tris[3 * tri + 2]]);
				V3 ctc = MT*abc;
				unsigned int cc = tex->Get(
					(int)(ctc[0] * tex->w),
					(int)(ctc[1] * tex->h));
				color.SetColor(cc);
			}
			// compute reflected ray at current intersection
			rO = O + ray*ct;
			M33 MN;
			MN.SetColumn(0, normals[tris[3 * tri + 0]]);
			MN.SetColumn(1, normals[tris[3 * tri + 1]]);
			MN.SetColumn(2, normals[tris[3 * tri + 2]]);
			V3 cn = (MN*abc).Normalized();
			// flip normal if back-facing so reflection goes outward
			if (cn * ray > 0.0f)
				cn = cn * -1.0f;
			rray = cn.Reflect(ray*-1.0f);
			t = ct;
		}
	}

	return (t != FLT_MAX);

}


// --- New methods ---

void TM::UpdateCenterOfMass() {
	centerOfMass = V3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < vertsN; i++)
		centerOfMass = centerOfMass + verts[i];
	centerOfMass = centerOfMass / (float)vertsN;

	if (normals) {
		for (int i = 0; i < vertsN; i++)
			normals[i] = normals[i].Normalized();
	}
}

void TM::TranslateTo(V3 position) {
	V3 delta = position - GetCenter();
	Translate(delta);
}

void TM::Scale(float factor) {
	V3 center = GetCenter();
	for (int i = 0; i < vertsN; i++)
		verts[i] = (verts[i] - center) * factor + center;
}

void TM::ProjectVertices(PPC *ppc) {
	if (!projverts)
		projverts = new V3[vertsN];
	for (int vi = 0; vi < vertsN; vi++) {
		if (!ppc->Project(verts[vi], projverts[vi])) {
			projverts[vi][2] = -1.0f;
		}
	}
}

void TM::SetTriangle(int index, unsigned int v0, unsigned int v1, unsigned int v2) {
	if (tris && index < trisN) {
		tris[index * 3 + 0] = v0;
		tris[index * 3 + 1] = v1;
		tris[index * 3 + 2] = v2;
	}
}

void TM::LoadRectangle(V3 center, V3 dimensions, V3 color) {
	V3 halfs = dimensions / 2.0f;

	if (verts) delete[] verts;
	if (colors) delete[] colors;
	if (tris) delete[] tris;
	if (normals) delete[] normals;

	vertsN = 8;
	verts = new V3[vertsN];
	colors = new V3[vertsN];
	trisN = 12;
	tris = new unsigned int[trisN * 3];
	normals = 0;
	projverts = 0;
	tcs = 0;
	tex = 0;

	verts[0] = V3(center[0] - halfs[0], center[1] + halfs[1], center[2] + halfs[2]);
	verts[1] = V3(center[0] - halfs[0], center[1] - halfs[1], center[2] + halfs[2]);
	verts[2] = V3(center[0] + halfs[0], center[1] - halfs[1], center[2] + halfs[2]);
	verts[3] = V3(center[0] + halfs[0], center[1] + halfs[1], center[2] + halfs[2]);
	verts[4] = V3(center[0] - halfs[0], center[1] + halfs[1], center[2] - halfs[2]);
	verts[5] = V3(center[0] - halfs[0], center[1] - halfs[1], center[2] - halfs[2]);
	verts[6] = V3(center[0] + halfs[0], center[1] - halfs[1], center[2] - halfs[2]);
	verts[7] = V3(center[0] + halfs[0], center[1] + halfs[1], center[2] - halfs[2]);

	for (int i = 0; i < vertsN; i++) colors[i] = color;

	SetTriangle(0, 0, 1, 2); SetTriangle(1, 0, 2, 3);   // front
	SetTriangle(2, 0, 3, 7); SetTriangle(3, 0, 7, 4);   // top
	SetTriangle(4, 1, 2, 6); SetTriangle(5, 1, 6, 5);   // bottom
	SetTriangle(6, 4, 5, 6); SetTriangle(7, 4, 6, 7);   // back
	SetTriangle(8, 0, 4, 5); SetTriangle(9, 0, 5, 1);   // left
	SetTriangle(10, 3, 6, 7); SetTriangle(11, 3, 2, 6); // right

	UpdateCenterOfMass();
}

void TM::LoadPlane(V3 center, V3 dimensions, V3 color) {
	V3 halfs = dimensions / 2.0f;

	if (verts) delete[] verts;
	if (colors) delete[] colors;
	if (normals) delete[] normals;
	if (tris) delete[] tris;
	if (tcs) delete[] tcs;

	vertsN = 4;
	verts = new V3[vertsN];
	colors = new V3[vertsN];
	normals = new V3[vertsN];
	tcs = new V3[vertsN];
	trisN = 2;
	tris = new unsigned int[trisN * 3];
	projverts = 0;
	tex = 0;

	verts[0] = V3(center[0] - halfs[0], center[1], center[2] + halfs[2]);
	verts[1] = V3(center[0] - halfs[0], center[1], center[2] - halfs[2]);
	verts[2] = V3(center[0] + halfs[0], center[1], center[2] - halfs[2]);
	verts[3] = V3(center[0] + halfs[0], center[1], center[2] + halfs[2]);

	for (int i = 0; i < vertsN; i++) colors[i] = color;
	for (int i = 0; i < vertsN; i++) normals[i] = V3(0.0f, 1.0f, 0.0f);

	tcs[0] = V3(0.0f, 1.0f, 0.0f);
	tcs[1] = V3(0.0f, 0.0f, 0.0f);
	tcs[2] = V3(1.0f, 0.0f, 0.0f);
	tcs[3] = V3(1.0f, 1.0f, 0.0f);

	SetTriangle(0, 2, 1, 0);
	SetTriangle(1, 3, 2, 0);

	UpdateCenterOfMass();
}

void TM::LoadRectangleWithNormals(V3 center, V3 dimensions, V3 color) {
	V3 h = dimensions / 2.0f;

	if (verts) delete[] verts;
	if (colors) delete[] colors;
	if (normals) delete[] normals;
	if (tris) delete[] tris;
	if (tcs) delete[] tcs;

	vertsN = 24; // 4 per face * 6 faces
	verts = new V3[vertsN];
	colors = new V3[vertsN];
	normals = new V3[vertsN];
	tcs = new V3[vertsN];
	trisN = 12;
	tris = new unsigned int[trisN * 3];
	projverts = 0;
	tex = 0;

	// 8 corner positions
	V3 c[8] = {
		V3(center[0] - h[0], center[1] + h[1], center[2] + h[2]), // 0: left top front
		V3(center[0] - h[0], center[1] - h[1], center[2] + h[2]), // 1: left bot front
		V3(center[0] + h[0], center[1] - h[1], center[2] + h[2]), // 2: right bot front
		V3(center[0] + h[0], center[1] + h[1], center[2] + h[2]), // 3: right top front
		V3(center[0] - h[0], center[1] + h[1], center[2] - h[2]), // 4: left top back
		V3(center[0] - h[0], center[1] - h[1], center[2] - h[2]), // 5: left bot back
		V3(center[0] + h[0], center[1] - h[1], center[2] - h[2]), // 6: right bot back
		V3(center[0] + h[0], center[1] + h[1], center[2] - h[2]), // 7: right top back
	};

	struct Face { int v[4]; V3 n; };
	Face faces[6] = {
		{{0, 1, 2, 3}, V3(0, 0, 1)},   // front +Z
		{{7, 6, 5, 4}, V3(0, 0, -1)},  // back -Z
		{{0, 3, 7, 4}, V3(0, 1, 0)},   // top +Y
		{{1, 5, 6, 2}, V3(0, -1, 0)},  // bottom -Y
		{{0, 4, 5, 1}, V3(-1, 0, 0)},  // left -X
		{{3, 2, 6, 7}, V3(1, 0, 0)},   // right +X
	};

	V3 uvs[4] = { V3(0,0,0), V3(1,0,0), V3(1,1,0), V3(0,1,0) };

	for (int f = 0; f < 6; f++) {
		int base = f * 4;
		for (int v = 0; v < 4; v++) {
			verts[base + v] = c[faces[f].v[v]];
			colors[base + v] = color;
			normals[base + v] = faces[f].n;
			tcs[base + v] = uvs[v];
		}
		SetTriangle(f * 2, base, base + 1, base + 2);
		SetTriangle(f * 2 + 1, base, base + 2, base + 3);
	}

	UpdateCenterOfMass();
}

void TM::LoadAABB(AABB aabb, V3 color) {
	V3 sz = aabb.GetSize();
	LoadRectangle(aabb.c0 + sz / 2.0f, sz, color);
}

void TM::DrawFilledPointLight(PPC *ppc, FrameBuffer *fb, V3 lightPos, float ka, float specularIntensity) {
	ProjectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {
		V3 pv0 = projverts[tris[3 * tri + 0]];
		V3 pv1 = projverts[tris[3 * tri + 1]];
		V3 pv2 = projverts[tris[3 * tri + 2]];
		if (pv0[2] < 0.0f || pv1[2] < 0.0f || pv2[2] < 0.0f)
			continue;

		V3 wp0 = verts[tris[3 * tri + 0]];
		V3 wp1 = verts[tris[3 * tri + 1]];
		V3 wp2 = verts[tris[3 * tri + 2]];

		AABB aabb(pv0);
		aabb.AddPoint(pv1);
		aabb.AddPoint(pv2);
		int left = (int)(aabb.c0[0] + .5f);
		int right = (int)(aabb.c1[0] - .5f);
		int top = (int)(aabb.c0[1] + .5f);
		int bottom = (int)(aabb.c1[1] - .5f);

		M33 pvs;
		pvs[0] = pv0; pvs[1] = pv1; pvs[2] = pv2;
		V3 pvzs = pvs.GetColumn(2);
		pvs.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		M33 pvsInv = pvs.Inverted();
		V3 zabc = pvsInv * pvzs;

		M33 colsm;
		colsm[0] = colors[tris[3 * tri + 0]];
		colsm[1] = colors[tris[3 * tri + 1]];
		colsm[2] = colors[tris[3 * tri + 2]];
		M33 rgbabc = (pvsInv * colsm).Transposed();

		M33 normsm;
		if (normals) {
			normsm[0] = normals[tris[3 * tri + 0]];
			normsm[1] = normals[tris[3 * tri + 1]];
			normsm[2] = normals[tris[3 * tri + 2]];
		}
		M33 normabc = (pvsInv * normsm).Transposed();

		M33 worldm;
		worldm[0] = wp0; worldm[1] = wp1; worldm[2] = wp2;
		M33 worldabc = (pvsInv * worldm).Transposed();

		M33 eeqs; SetEEQs(pvs, eeqs);

		for (int v = top; v <= bottom; v++) {
			for (int u = left; u <= right; u++) {
				V3 pixcv(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 sdv = eeqs * pixcv;
				if (sdv[0] < 0.0f || sdv[1] < 0.0f || sdv[2] < 0.0f)
					continue;
				float currz = zabc * pixcv;
				if (fb->IsFarther(u, v, currz))
					continue;

				V3 currcol = rgbabc * pixcv;
				V3 currn = (normabc * pixcv).Normalized();
				V3 currworld = worldabc * pixcv;

				// flip back-facing normals so lighting works correctly
				if (currn * (ppc->C - currworld) < 0.0f)
					currn = currn * -1.0f;

				V3 L = (lightPos - currworld).Normalized();
				currcol = currcol.Light(currn, L, ka);

				// specular highlight
				if (specularIntensity > 0.0f) {
					V3 viewDir = (ppc->C - currworld).Normalized();
					float spec = currn.Reflect(L) * viewDir;
					if (spec < 0.0f) spec = 0.0f;
					spec = powf(spec, specularIntensity);
					float cutoff = 0.7f;
					if (spec >= cutoff) {
						currcol = V3(1.0f, 1.0f, 1.0f) * spec + currcol * (1.0f - spec);
					}
				}

				fb->SetSafe(u, v, currcol.GetColor());
				fb->SetSafeZB(u, v, currz);
			}
		}
	}
}

void TM::DrawFilledPointLightShadow(PPC *ppc, FrameBuffer *fb, V3 lightPos, float ka, float specularIntensity,
	PPC *lightPPC, FrameBuffer *shadowFB, float shadowEps) {
	ProjectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {
		V3 pv0 = projverts[tris[3 * tri + 0]];
		V3 pv1 = projverts[tris[3 * tri + 1]];
		V3 pv2 = projverts[tris[3 * tri + 2]];
		if (pv0[2] < 0.0f || pv1[2] < 0.0f || pv2[2] < 0.0f)
			continue;

		V3 wp0 = verts[tris[3 * tri + 0]];
		V3 wp1 = verts[tris[3 * tri + 1]];
		V3 wp2 = verts[tris[3 * tri + 2]];

		AABB aabb(pv0);
		aabb.AddPoint(pv1);
		aabb.AddPoint(pv2);
		int left = (int)(aabb.c0[0] + .5f);
		int right = (int)(aabb.c1[0] - .5f);
		int top = (int)(aabb.c0[1] + .5f);
		int bottom = (int)(aabb.c1[1] - .5f);

		M33 pvs;
		pvs[0] = pv0; pvs[1] = pv1; pvs[2] = pv2;
		V3 pvzs = pvs.GetColumn(2);
		pvs.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		M33 pvsInv = pvs.Inverted();
		V3 zabc = pvsInv * pvzs;

		M33 colsm;
		colsm[0] = colors[tris[3 * tri + 0]];
		colsm[1] = colors[tris[3 * tri + 1]];
		colsm[2] = colors[tris[3 * tri + 2]];
		M33 rgbabc = (pvsInv * colsm).Transposed();

		M33 normsm;
		if (normals) {
			normsm[0] = normals[tris[3 * tri + 0]];
			normsm[1] = normals[tris[3 * tri + 1]];
			normsm[2] = normals[tris[3 * tri + 2]];
		}
		M33 normabc = (pvsInv * normsm).Transposed();

		M33 worldm;
		worldm[0] = wp0; worldm[1] = wp1; worldm[2] = wp2;
		M33 worldabc = (pvsInv * worldm).Transposed();

		M33 eeqs; SetEEQs(pvs, eeqs);

		for (int v = top; v <= bottom; v++) {
			for (int u = left; u <= right; u++) {
				V3 pixcv(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 sdv = eeqs * pixcv;
				if (sdv[0] < 0.0f || sdv[1] < 0.0f || sdv[2] < 0.0f)
					continue;
				float currz = zabc * pixcv;
				if (fb->IsFarther(u, v, currz))
					continue;

				V3 currcol = rgbabc * pixcv;
				V3 currn = (normabc * pixcv).Normalized();
				V3 currworld = worldabc * pixcv;

				// flip back-facing normals so lighting works correctly
				if (currn * (ppc->C - currworld) < 0.0f)
					currn = currn * -1.0f;

				V3 L = (lightPos - currworld).Normalized();
				currcol = currcol.Light(currn, L, ka);

				// specular highlight
				if (specularIntensity > 0.0f) {
					V3 viewDir = (ppc->C - currworld).Normalized();
					float spec = currn.Reflect(L) * viewDir;
					if (spec < 0.0f) spec = 0.0f;
					spec = powf(spec, specularIntensity);
					float cutoff = 0.7f;
					if (spec >= cutoff) {
						currcol = V3(1.0f, 1.0f, 1.0f) * spec + currcol * (1.0f - spec);
					}
				}

				// shadow test: project world point into light camera
				V3 shadowUV;
				if (lightPPC->Project(currworld, shadowUV)) {
					int su = (int)shadowUV[0];
					int sv = (int)shadowUV[1];
					if (su >= 0 && su < shadowFB->w && sv >= 0 && sv < shadowFB->h) {
						float lightZ = shadowFB->GetZB(su, sv);
						if (shadowUV[2] < lightZ - shadowEps) {
							// in shadow — ambient only
							currcol = currcol * ka;
						}
					}
				}

				fb->SetSafe(u, v, currcol.GetColor());
				fb->SetSafeZB(u, v, currz);
			}
		}
	}
}

void TM::DrawFilledSoftShadow(PPC *ppc, FrameBuffer *fb, V3 lightPos, float ka, float specularIntensity,
	PPC **lightPPCs, FrameBuffer **shadowFBs, int numSamples, float shadowEps) {
	ProjectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {
		V3 pv0 = projverts[tris[3 * tri + 0]];
		V3 pv1 = projverts[tris[3 * tri + 1]];
		V3 pv2 = projverts[tris[3 * tri + 2]];
		if (pv0[2] < 0.0f || pv1[2] < 0.0f || pv2[2] < 0.0f)
			continue;

		V3 wp0 = verts[tris[3 * tri + 0]];
		V3 wp1 = verts[tris[3 * tri + 1]];
		V3 wp2 = verts[tris[3 * tri + 2]];

		AABB aabb(pv0);
		aabb.AddPoint(pv1);
		aabb.AddPoint(pv2);
		int left = (int)(aabb.c0[0] + .5f);
		int right = (int)(aabb.c1[0] - .5f);
		int top = (int)(aabb.c0[1] + .5f);
		int bottom = (int)(aabb.c1[1] - .5f);

		M33 pvs;
		pvs[0] = pv0; pvs[1] = pv1; pvs[2] = pv2;
		V3 pvzs = pvs.GetColumn(2);
		pvs.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		M33 pvsInv = pvs.Inverted();
		V3 zabc = pvsInv * pvzs;

		M33 colsm;
		colsm[0] = colors[tris[3 * tri + 0]];
		colsm[1] = colors[tris[3 * tri + 1]];
		colsm[2] = colors[tris[3 * tri + 2]];
		M33 rgbabc = (pvsInv * colsm).Transposed();

		M33 normsm;
		if (normals) {
			normsm[0] = normals[tris[3 * tri + 0]];
			normsm[1] = normals[tris[3 * tri + 1]];
			normsm[2] = normals[tris[3 * tri + 2]];
		}
		M33 normabc = (pvsInv * normsm).Transposed();

		// Perspective-correct world position interpolation:
		// Interpolate (world * z) linearly in screen space, then divide by interpolated z
		M33 worldOverWm;
		worldOverWm[0] = wp0 * pvzs[0];
		worldOverWm[1] = wp1 * pvzs[1];
		worldOverWm[2] = wp2 * pvzs[2];
		M33 worldOverWabc = (pvsInv * worldOverWm).Transposed();

		M33 eeqs; SetEEQs(pvs, eeqs);

		for (int v = top; v <= bottom; v++) {
			for (int u = left; u <= right; u++) {
				V3 pixcv(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 sdv = eeqs * pixcv;
				if (sdv[0] < 0.0f || sdv[1] < 0.0f || sdv[2] < 0.0f)
					continue;
				float currz = zabc * pixcv;
				if (fb->IsFarther(u, v, currz))
					continue;

				V3 baseCol = rgbabc * pixcv;
				V3 currn = (normabc * pixcv).Normalized();
				V3 currworld = (worldOverWabc * pixcv) / currz;

				if (currn * (ppc->C - currworld) < 0.0f)
					currn = currn * -1.0f;

				V3 L = (lightPos - currworld).Normalized();
				V3 litCol = baseCol.Light(currn, L, ka);

				// specular
				if (specularIntensity > 0.0f) {
					V3 viewDir = (ppc->C - currworld).Normalized();
					float spec = currn.Reflect(L) * viewDir;
					if (spec < 0.0f) spec = 0.0f;
					spec = powf(spec, specularIntensity);
					float cutoff = 0.7f;
					if (spec >= cutoff) {
						litCol = V3(1.0f, 1.0f, 1.0f) * spec + litCol * (1.0f - spec);
					}
				}

				// slope-scaled shadow bias, capped to prevent detachment
				float cosTheta = fabsf(currn * L);
				if (cosTheta < 0.05f) cosTheta = 0.05f;
				float bias = shadowEps / cosTheta;
				if (bias > shadowEps * 4.0f) bias = shadowEps * 4.0f;

				// soft shadow: test visibility against all k*k shadow maps with 2x2 PCF
				float visibility = 0.0f;
				for (int si = 0; si < numSamples; si++) {
					V3 shadowUV;
					if (lightPPCs[si]->Project(currworld, shadowUV)) {
						float sf = shadowUV[0] - 0.5f;
						float tf = shadowUV[1] - 0.5f;
						int s0 = (int)floorf(sf);
						int t0 = (int)floorf(tf);
						float fu = sf - s0;
						float fv = tf - t0;
						int sw = shadowFBs[si]->w;
						int sh = shadowFBs[si]->h;
						float subvis = 0.0f;
						for (int dv = 0; dv <= 1; dv++) {
							for (int du = 0; du <= 1; du++) {
								int px = s0 + du;
								int py = t0 + dv;
								float wt = (du ? fu : 1.0f - fu) * (dv ? fv : 1.0f - fv);
								if (px >= 0 && px < sw && py >= 0 && py < sh) {
									float lightZ = shadowFBs[si]->GetZB(px, py);
									if (lightZ == 0.0f || shadowUV[2] >= lightZ - bias) {
										subvis += wt;
									}
								} else {
									subvis += wt;
								}
							}
						}
						visibility += subvis;
					} else {
						visibility += 1.0f;
					}
				}
				visibility /= (float)numSamples;

				// blend: full shadow = ambient only, full lit = litCol
				V3 ambientCol = baseCol * ka;
				V3 currcol = ambientCol * (1.0f - visibility) + litCol * visibility;

				fb->SetSafe(u, v, currcol.GetColor());
				fb->SetSafeZB(u, v, currz);
			}
		}
	}
}

void TM::DrawTextured(PPC *ppc, FrameBuffer *fb, FrameBuffer *texFB, int filterMode, int tileMode) {
	ProjectVertices(ppc);

	M33 abc = M33::FromColumns(ppc->a, ppc->b, ppc->c);

	for (int tri = 0; tri < trisN; tri++) {
		V3 pv0 = projverts[tris[3 * tri + 0]];
		V3 pv1 = projverts[tris[3 * tri + 1]];
		V3 pv2 = projverts[tris[3 * tri + 2]];
		if (pv0[2] < 0.0f || pv1[2] < 0.0f || pv2[2] < 0.0f)
			continue;

		// perspective-correct interpolation coefficients
		M33 Q = M33::FromColumns(
			verts[tris[3 * tri + 0]] - ppc->C,
			verts[tris[3 * tri + 1]] - ppc->C,
			verts[tris[3 * tri + 2]] - ppc->C
		).Inverted() * abc;

		// texture coordinates for this triangle's vertices
		V3 tc0 = tcs[tris[3 * tri + 0]];
		V3 tc1 = tcs[tris[3 * tri + 1]];
		V3 tc2 = tcs[tris[3 * tri + 2]];

		V3 texX(tc0[0], tc1[0], tc2[0]);
		V3 texY(tc0[1], tc1[1], tc2[1]);

		M33 QT = Q.Transposed();
		V3 txABC = QT * texX;
		V3 tyABC = QT * texY;
		V3 DEF = Q.ColumnSums();

		AABB aabb(pv0);
		aabb.AddPoint(pv1);
		aabb.AddPoint(pv2);
		int left = max(0, (int)(aabb.c0[0] + .5f));
		int right = min(fb->w - 1, (int)(aabb.c1[0] - .5f));
		int top = max(0, (int)(aabb.c0[1] + .5f));
		int bottom = min(fb->h - 1, (int)(aabb.c1[1] - .5f));

		M33 pvs;
		pvs[0] = pv0; pvs[1] = pv1; pvs[2] = pv2;
		V3 pvzs = pvs.GetColumn(2);
		pvs.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		V3 zabc = pvs.Inverted() * pvzs;

		M33 eeqs; SetEEQs(pvs, eeqs);

		for (int v = top; v <= bottom; v++) {
			for (int u = left; u <= right; u++) {
				V3 pixcv(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 sdv = eeqs * pixcv;
				if (sdv[0] < 0.0f || sdv[1] < 0.0f || sdv[2] < 0.0f)
					continue;
				float currz = zabc * pixcv;
				if (fb->IsFarther(u, v, currz))
					continue;

				V3 uv1((float)u, (float)v, 1.0f);
				float denom = DEF * uv1;
				float tx = (txABC * uv1) / denom;
				float ty = (tyABC * uv1) / denom;

				V3 texCol = texFB->GetColor(tx, ty, tileMode != 0, filterMode != 0);
				fb->SetSafe(u, v, texCol.GetColor());
				fb->SetSafeZB(u, v, currz);
			}
		}
	}
}

void TM::DrawFilledEnvMap(PPC *ppc, FrameBuffer *fb, CubeMap *map) {
	if (!normals || !map) return;
	ProjectVertices(ppc);

	M33 abc = M33::FromColumns(ppc->a, ppc->b, ppc->c);

	for (int tri = 0; tri < trisN; tri++) {
		V3 pv0 = projverts[tris[3 * tri + 0]];
		V3 pv1 = projverts[tris[3 * tri + 1]];
		V3 pv2 = projverts[tris[3 * tri + 2]];
		if (pv0[2] < 0.0f || pv1[2] < 0.0f || pv2[2] < 0.0f)
			continue;

		// perspective-correct normal interpolation
		M33 Q = M33::FromColumns(
			verts[tris[3 * tri + 0]] - ppc->C,
			verts[tris[3 * tri + 1]] - ppc->C,
			verts[tris[3 * tri + 2]] - ppc->C
		).Inverted() * abc;

		V3 n0 = normals[tris[3 * tri + 0]];
		V3 n1 = normals[tris[3 * tri + 1]];
		V3 n2 = normals[tris[3 * tri + 2]];

		M33 QT = Q.Transposed();
		V3 nxABC = QT * V3(n0[0], n1[0], n2[0]);
		V3 nyABC = QT * V3(n0[1], n1[1], n2[1]);
		V3 nzABC = QT * V3(n0[2], n1[2], n2[2]);
		V3 DEF = Q.ColumnSums();

		AABB aabb(pv0);
		aabb.AddPoint(pv1);
		aabb.AddPoint(pv2);
		int left = max(0, (int)(aabb.c0[0] + .5f));
		int right = min(fb->w - 1, (int)(aabb.c1[0] - .5f));
		int top = max(0, (int)(aabb.c0[1] + .5f));
		int bottom = min(fb->h - 1, (int)(aabb.c1[1] - .5f));

		M33 pvs;
		pvs[0] = pv0; pvs[1] = pv1; pvs[2] = pv2;
		V3 pvzs = pvs.GetColumn(2);
		pvs.SetColumn(2, V3(1.0f, 1.0f, 1.0f));
		V3 zabc = pvs.Inverted() * pvzs;
		M33 eeqs; SetEEQs(pvs, eeqs);

		for (int v = top; v <= bottom; v++) {
			for (int u = left; u <= right; u++) {
				V3 pixcv(.5f + (float)u, .5f + (float)v, 1.0f);
				V3 sdv = eeqs * pixcv;
				if (sdv[0] < 0.0f || sdv[1] < 0.0f || sdv[2] < 0.0f)
					continue;
				float currz = zabc * pixcv;
				if (fb->IsFarther(u, v, currz))
					continue;

				V3 uv1((float)u, (float)v, 1.0f);
				float denom = DEF * uv1;
				V3 N = V3(
					(nxABC * uv1) / denom,
					(nyABC * uv1) / denom,
					(nzABC * uv1) / denom
				).Normalized();

				V3 eyeRay = (ppc->Unproject(u, v, currz) - ppc->C).Normalized();
				V3 RR = N.Reflect(eyeRay);
				V3 envColor = map->Lookup(RR);

				fb->SetSafe(u, v, envColor.GetColor());
				fb->SetSafeZB(u, v, currz);
			}
		}
	}
}
