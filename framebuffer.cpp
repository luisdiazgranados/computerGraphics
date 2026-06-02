#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif
#include "framebuffer.h"
#include "color.h"
#include "font.h"
#include "aabb.h"
#include "math.h"
#include "scene.h"

#include <tiffio.h>

using namespace std;

#include <iostream>
#include <fstream>
#include <strstream>
#include <algorithm>
#include <cstring>


FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h) : 
	Fl_Gl_Window(u0, v0, _w, _h, 0) {

	w = _w;
	h = _h;
	pix = new unsigned int[w*h];
	zb = new float[w*h];
	ishw = 0;
	needInitHW = 1;
}

void FrameBuffer::draw() {

	if (!ishw)
		glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	else {
		if (needInitHW) {
			scene->InitHWRendering();
			needInitHW = 0;
		}
		scene->RenderHW();
		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
	}
}

int FrameBuffer::handle(int event) {

	switch (event)
	{
	case FL_KEYBOARD: {
		KeyboardHandle();
		return 0;
	}
	case FL_MOVE: {
		int u = Fl::event_x();
		int v = Fl::event_y();
		if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
			return 0;
		V3 cv; cv.SetColor(Get(u, v));
		cerr << u << " " << v << " " << "c: " << cv << "                  \r";
		return 0;
	}
	default:
		return 0;
	}
	return 0;
}

void FrameBuffer::KeyboardHandle() {

	int key = Fl::event_key();
	switch (key) {
	case FL_Left: {
		cerr << "INFO: pressed left arrow key";
		cerr << endl << endl;
		scene->Render();
		break;
	}
	case 'a': {
		cerr << "INFO: pan camera" << endl;
		break;
	}
	case '1': {
		cerr << "INFO: mesh lighting demo" << endl;
		scene->RenderMeshLighting();
		break;
	}
	case '2': {
		cerr << "INFO: shadow map demo" << endl;
		scene->RenderShadowMap();
		break;
	}
	case '3': {
		cerr << "INFO: textured rendering demo" << endl;
		scene->RenderTextured();
		break;
	}
	case '4': {
		cerr << "INFO: environment mapping demo" << endl;
		scene->RenderEnvMap();
		break;
	}
	case '5': {
		cerr << "INFO: soft shadow demo" << endl;
		scene->RenderSoftShadow();
		break;
	}
	case '6': {
		cerr << "INFO: HW shadow mapping" << endl;
		scene->hwRenderMode = 1;
		scene->hwfb->show();
		scene->hwfb->redraw();
		break;
	}
	case '7': {
		cerr << "INFO: HW projective texture mapping" << endl;
		scene->hwRenderMode = 2;
		scene->hwfb->show();
		scene->hwfb->redraw();
		break;
	}
	case '8': {
		scene->SaveCameraKeyframe();
		break;
	}
	case '9': {
		scene->PlayCameraPath();
		break;
	}
	case '0': {
		scene->ClearCameraPath();
		break;
	}
	case 'r': {
		cerr << "INFO: ray tracing" << endl;
		scene->RayTrace();
		break;
	}
	default:
		cerr << "INFO: do not understand keypress" << endl;
		return;
	}

}

// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff(char* fname) {
	TIFF* in = TIFFOpen(fname, "r");
	if (in == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	if (w != width || h != height) {
		w = width;
		h = height;
		delete[] pix;
		pix = new unsigned int[w*h];
		size(w, h);
		glFlush();
		glFlush();
	}

	if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0) {
		cerr << "failed to load " << fname << endl;
	}

	TIFFClose(in);
}

// save as tiff image
void FrameBuffer::SaveAsTiff(char *fname) {

	TIFF* out = TIFFOpen(fname, "w");

	if (out == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

	for (uint32 row = 0; row < (unsigned int)h; row++) {
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}



void FrameBuffer::Set(unsigned int color) {

	for (int uv = 0; uv < w*h; uv++)
		pix[uv] = color;

}

void FrameBuffer::SetZB(float z0) {

	for (int uv = 0; uv < w*h; uv++)
		zb[uv] = z0;

}


unsigned int FrameBuffer::Get(int u, int v) {

	if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
		return 0XFF000000;
	return pix[(h - 1 - v)*w + u];

}

float FrameBuffer::GetZB(int u, int v) {

	return zb[(h - 1 - v)*w + u];

}


void FrameBuffer::SetSafe(int u, int v,
	unsigned int color) {

	if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
		return;

	Set(u, v, color);

}

void FrameBuffer::SetSafeZB(int u, int v, float z) {

	if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
		return;

	SetZB(u, v, z);

}


void FrameBuffer::Set(int u, int v, unsigned int color) {

	pix[(h - 1 - v)*w + u] = color;

}

void FrameBuffer::SetZB(int u, int v, float z) {

	zb[(h - 1 - v)*w + u] = z;

}


void FrameBuffer::SetChecker(int cw, unsigned int col0,
	unsigned int col1) {

	for (int v = 0; v < h; v++) {
		for (int u = 0; u < w; u++) {
			int cu, cv;
			cu = u / cw;
			cv = v / cw;
			if ((cu+cv)%2)
				Set(u, v, col0);
			else
				Set(u, v, col1);
		}
	}


}


void FrameBuffer::DrawPoint2D(V3 P, int psize, unsigned int color) {


	int up = (int)P[0];
	int vp = (int)P[1];

	for (int v = vp - psize / 2; v <= vp + psize / 2; v++) {
		for (int u = up - psize / 2; u <= up + psize / 2; u++) {

			if (IsFarther(u, v, P[2]))
				continue;
			SetSafe(u, v, color);
			SetSafeZB(u, v, P[2]);

		}
	}


}


void FrameBuffer::DrawPoint3D(V3 P, PPC *ppc, int psize,
	unsigned int color) {

	V3 Pp;
	if (!ppc->Project(P, Pp))
		return;

	DrawPoint2D(Pp, psize, color);

}

void FrameBuffer::Draw3DSegment(unsigned int color, PPC *ppc,
	V3 V0, V3 V1) {

	V3 pV0, pV1;
	if (!ppc->Project(V0, pV0))
		return;
	if (!ppc->Project(V1, pV1))
		return;
	Draw2DSegment(color, pV0, pV1);
}

void FrameBuffer::Draw3DSegment(V3 C0, V3 C1, PPC *ppc,
	V3 V0, V3 V1) {

	V3 pV0, pV1;
	if (!ppc->Project(V0, pV0))
		return;
	if (!ppc->Project(V1, pV1))
		return;
	Draw2DSegment(C0, C1, pV0, pV1);
}


void FrameBuffer::Draw2DSegment(unsigned int color, V3 pV0, V3 pV1) {

	V3 v2d0 = pV0;
	V3 v2d1 = pV1;
	v2d0[2] = 0.0f;
	v2d1[2] = 0.0f;
	int pixn = (int)((v2d0-v2d1).Length() + 2);
	for (int si = 0; si < pixn; si++) {
		V3 currP = pV0 + (pV1 - pV0)*(float)si / (float)(pixn - 1);
		SetSafe((int) currP[0], (int) currP[1], color);
	}

}

int FrameBuffer::IsFarther(int u, int v, float z) {

	if (u < 0 || v < 0 || u > w - 1 || v > h - 1)
		return 1;
	float currz = GetZB(u, v);
	return (z < currz);

}

void FrameBuffer::Draw2DSegment(V3 C0, V3 C1, V3 pV0, V3 pV1) {

	V3 v2d0 = pV0;
	V3 v2d1 = pV1;
	v2d0[2] = 0.0f;
	v2d1[2] = 0.0f;
	int pixn = (int)((v2d0-v2d1).Length() + 2);
	for (int si = 0; si < pixn; si++) {
		V3 currP = pV0 + (pV1 - pV0)*(float)si / (float)(pixn - 1);
		V3 currC = C0 + (C1 - C0)*(float)si / (float)(pixn - 1);
		unsigned int color = currC.GetColor();
		int u = (int)currP[0];
		int v = (int)currP[1];
		if (IsFarther(u, v, currP[2]))
			continue;
		SetSafe(u, v, color);
		SetSafeZB(u, v, currP[2]);
	}

}


void FrameBuffer::DrawFBPointCloud(FrameBuffer *pcfb, PPC *pcppc, PPC *ppc) {

	for (int v = 0; v < pcfb->h; v++) {
		for (int u = 0; u < pcfb->w; u++) {
			float currz = pcfb->GetZB(u, v);
			if (currz == 0.0f)
				continue;
			V3 P = pcppc->Unproject(u, v, currz);
			DrawPoint3D(P, ppc, 1, pcfb->Get(u, v));
		}
	}

}


// --- New drawing primitives ---

void FrameBuffer::DrawRect(int u, int v, int width, int height, unsigned int color) {
	int uMax = u + width;
	int vMax = v + height;
	if (u < 0) u = 0;
	if (uMax > w) uMax = w;
	if (v < 0) v = 0;
	if (vMax > h) vMax = h;
	for (int y = v; y < vMax; y++) {
		for (int x = u; x < uMax; x++) {
			Set(x, y, color);
		}
	}
}

void FrameBuffer::DrawCircle(int u, int v, int radius, unsigned int color) {
	int uMin = u - radius;
	int uMax = u + radius;
	int vMin = v - radius;
	int vMax = v + radius;
	if (uMin < 0) uMin = 0;
	if (uMax >= w) uMax = w - 1;
	if (vMin < 0) vMin = 0;
	if (vMax >= h) vMax = h - 1;
	int squareRadius = radius * radius;
	for (int y = vMin; y <= vMax; y++) {
		int dy = y - v;
		int squareDy = dy * dy;
		for (int x = uMin; x <= uMax; x++) {
			int dx = x - u;
			if (dx * dx + squareDy <= squareRadius) {
				Set(x, y, color);
			}
		}
	}
}

void FrameBuffer::DrawLine(int u0, int v0, int u1, int v1, unsigned int color) {
	float du = (float)(u1 - u0);
	float dv = (float)(v1 - v0);
	float steps = max(fabsf(du), fabsf(dv));
	if (steps == 0.0f) {
		SetSafe(u0, v0, color);
		return;
	}
	du /= steps;
	dv /= steps;
	float u = (float)u0, v = (float)v0;
	for (int step = 0; step <= (int)steps; step++) {
		SetSafe((int)u, (int)v, color);
		u += du;
		v += dv;
	}
}

void FrameBuffer::DrawLine(int u0, int v0, int u1, int v1, V3 c0, V3 c1) {
	float du = (float)(u1 - u0);
	float dv = (float)(v1 - v0);
	float steps = max(fabsf(du), fabsf(dv));
	if (steps == 0.0f) {
		SetSafe(u0, v0, c0.GetColor());
		return;
	}
	du /= steps;
	dv /= steps;
	float u = (float)u0, v = (float)v0;
	for (int step = 0; step <= (int)steps; step++) {
		V3 cc = c0.Interpolate(c1, (float)step / steps);
		SetSafe((int)u, (int)v, cc.GetColor());
		u += du;
		v += dv;
	}
}

void FrameBuffer::DrawTriangle2D(int u0, int v0, int u1, int v1, int u2, int v2, unsigned int color) {
	// edge equation coefficients
	V3 ea, eb, ec;
	ea[0] = (float)(v1 - v0); ea[1] = (float)(v2 - v1); ea[2] = (float)(v0 - v2);
	eb[0] = (float)(-u1 + u0); eb[1] = (float)(-u2 + u1); eb[2] = (float)(-u0 + u2);
	ec[0] = (float)(-u0*v1 + v0*u1); ec[1] = (float)(-u1*v2 + v1*u2); ec[2] = (float)(-u2*v0 + v2*u0);

	// orient edges
	if (ea[0] * u2 + eb[0] * v2 + ec[0] < 0) { ea[0] = -ea[0]; eb[0] = -eb[0]; ec[0] = -ec[0]; }
	if (ea[1] * u0 + eb[1] * v0 + ec[1] < 0) { ea[1] = -ea[1]; eb[1] = -eb[1]; ec[1] = -ec[1]; }
	if (ea[2] * u1 + eb[2] * v1 + ec[2] < 0) { ea[2] = -ea[2]; eb[2] = -eb[2]; ec[2] = -ec[2]; }

	int left = min({u0, u1, u2});
	int right = max({u0, u1, u2});
	int top = min({v0, v1, v2});
	int bottom = max({v0, v1, v2});
	if (left < 0) left = 0;
	if (right >= w) right = w - 1;
	if (top < 0) top = 0;
	if (bottom >= h) bottom = h - 1;

	for (int y = top; y <= bottom; y++) {
		for (int x = left; x <= right; x++) {
			float px = x + 0.5f, py = y + 0.5f;
			if (ea[0]*px + eb[0]*py + ec[0] >= 0 &&
				ea[1]*px + eb[1]*py + ec[1] >= 0 &&
				ea[2]*px + eb[2]*py + ec[2] >= 0) {
				Set(x, y, color);
			}
		}
	}
}

// --- Font rendering ---

void FrameBuffer::DrawChar(int u, int v, int scale, char ch, unsigned int color) {
	const unsigned char *bits = FontGetChar(ch);
	int startU = u;
	for (unsigned int row = 0; row < FontSize(); row++) {
		unsigned char byte = bits[row];
		for (unsigned int col = 0; col < FontSize(); col++) {
			if (byte & 1) {
				DrawRect(u, v, scale, scale, color);
			}
			byte >>= 1;
			u += scale;
		}
		u = startU;
		v += scale;
	}
}

void FrameBuffer::DrawString(int u, int v, int scale, const char* str, unsigned int color) {
	int charU = u;
	int charV = v;
	for (const char *ch = str; *ch; ch++) {
		if (*ch == '\n') {
			charU = u;
			charV += FontSize() * scale;
		} else {
			DrawChar(charU, charV, scale, *ch, color);
			charU += FontSize() * scale;
		}
	}
}

// --- Color queries ---

V3 FrameBuffer::GetColorV3(int u, int v) {
	V3 ret;
	ret.SetColor(Get(u, v));
	return ret;
}

V3 FrameBuffer::GetColor(float x, float y, bool repeat, bool bilinear) {
	int u, v;
	if (repeat) {
		float halfX = x / 2.0f;
		float halfY = y / 2.0f;
		u = (int)(w * 2 * fabsf(halfX - floorf(halfX + 0.5f)));
		v = (int)(h * 2 * fabsf(halfY - floorf(halfY + 0.5f)));
	} else {
		u = (int)(x * (w - 1));
		v = (int)(y * (h - 1));
		u = ((u % w) + w) % w;
		v = ((v % h) + h) % h;
	}
	if (bilinear) {
		V3 tl = GetColorV3(u - 1, v - 1);
		V3 l = GetColorV3(u - 1, v);
		V3 t = GetColorV3(u, v - 1);
		V3 c = GetColorV3(u, v);
		return (tl + l + t + c) / 4.0f;
	}
	return GetColorV3(u, v);
}

V3 FrameBuffer::GetColorBilinear(float x, float y) {
	int cu = (int)floorf(x + 0.5f);
	int cv = (int)floorf(y + 0.5f);
	if (cu < 1) cu = 1; else if (cu >= w) cu = w - 1;
	if (cv < 1) cv = 1; else if (cv >= h) cv = h - 1;

	V3 tl = GetColorV3(cu - 1, cv - 1);
	V3 bl = GetColorV3(cu - 1, cv);
	V3 tr = GetColorV3(cu, cv - 1);
	V3 br = GetColorV3(cu, cv);

	float dx = cu - x + 0.5f;
	float dy = cv - y + 0.5f;
	return tl * (dx * dy) + bl * (dx * (1.0f - dy)) +
		tr * ((1.0f - dx) * dy) + br * ((1.0f - dx) * (1.0f - dy));
}

// --- Z-buffer visualization ---

void FrameBuffer::DrawZBuffer() {
	DrawZBuffer(this);
}

void FrameBuffer::DrawZBuffer(FrameBuffer *other) {
	int width = min(w, other->w);
	int height = min(h, other->h);
	for (int v = 0; v < height; v++) {
		for (int u = 0; u < width; u++) {
			SetSafe(u, v, ColorFromInverseZ(other->GetZB(u, v)));
		}
	}
}

// --- Copy ---

void FrameBuffer::Copy(FrameBuffer *other) {
	int width = min(w, other->w);
	int height = min(h, other->h);
	for (int v = 0; v < height; v++) {
		for (int u = 0; u < width; u++) {
			Set(u, v, other->Get(u, v));
			SetZB(u, v, other->GetZB(u, v));
		}
	}
}

// --- Camera visualization ---

void FrameBuffer::DrawCamera(PPC *viewPPC, PPC *drawnPPC) {
	float f = viewPPC->GetF();
	float scf = 5.0f / f;

	V3 pC;
	if (!viewPPC->Project(drawnPPC->C, pC))
		return;

	DrawPoint3D(drawnPPC->C, viewPPC, 5, 0xFFFFFFFF);

	V3 corners[4], pCorners[4];
	corners[0] = drawnPPC->C + drawnPPC->c * scf;
	corners[1] = drawnPPC->C + (drawnPPC->c + drawnPPC->a * (float)drawnPPC->w) * scf;
	corners[2] = drawnPPC->C + (drawnPPC->c + drawnPPC->a * (float)drawnPPC->w + drawnPPC->b * (float)drawnPPC->h) * scf;
	corners[3] = drawnPPC->C + (drawnPPC->c + drawnPPC->b * (float)drawnPPC->h) * scf;

	bool proj[4];
	for (int i = 0; i < 4; i++)
		proj[i] = viewPPC->Project(corners[i], pCorners[i]);

	// draw frustum edges
	for (int i = 0; i < 4; i++) {
		if (proj[i])
			Draw3DSegment(0xFFFFFFFF, viewPPC, drawnPPC->C, corners[i]);
	}
	// draw image rectangle
	for (int i = 0; i < 4; i++) {
		int j = (i + 1) % 4;
		if (proj[i] && proj[j])
			Draw3DSegment(0xFFFFFFFF, viewPPC, corners[i], corners[j]);
	}
}

// --- AABB visualization ---

void FrameBuffer::DrawAABB(PPC *ppc, AABB aabb, unsigned int color) {
	V3 pts[8], ppts[8];
	pts[0] = aabb.c0;
	pts[1] = V3(aabb.c0[0], aabb.c0[1], aabb.c1[2]);
	pts[2] = V3(aabb.c0[0], aabb.c1[1], aabb.c0[2]);
	pts[3] = V3(aabb.c0[0], aabb.c1[1], aabb.c1[2]);
	pts[4] = V3(aabb.c1[0], aabb.c0[1], aabb.c0[2]);
	pts[5] = V3(aabb.c1[0], aabb.c0[1], aabb.c1[2]);
	pts[6] = V3(aabb.c1[0], aabb.c1[1], aabb.c0[2]);
	pts[7] = aabb.c1;

	bool ok = true;
	for (int i = 0; i < 8; i++)
		ok = ok && ppc->Project(pts[i], ppts[i]);
	if (!ok) return;

	// 12 edges of a box
	int edges[][2] = { {0,1},{0,2},{0,4},{1,3},{1,5},{2,3},{2,6},{3,7},{4,5},{4,6},{5,7},{6,7} };
	for (int i = 0; i < 12; i++)
		Draw3DSegment(color, ppc, pts[edges[i][0]], pts[edges[i][1]]);
}
