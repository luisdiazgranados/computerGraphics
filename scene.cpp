#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include "scene.h"
#include "v3.h"
#include "m33.h"
#include "ppc.h"
#include "TM.h"
#include "cubemap.h"

Scene *scene;


#include <iostream>
#include <fstream>
#include <strstream>
#include <thread>
#include <vector>

using namespace std;


Scene::Scene() {

	scene = this; // set global pointer early so callbacks work during construction

	int u0 = 20;
	int v0 = 40;
	int h = 400;
	int w = 600;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->position(u0, v0);
	fb->label("First Person View SW");
	fb->show();
	fb->redraw();

	fb3 = new FrameBuffer(u0, v0, w, h);
	fb3->position(u0+w+u0, v0);
	fb3->label("Third Person View");
	fb3->show();
	fb3->redraw();

	fb3->hide();
	hwfb = new FrameBuffer(u0, v0, w, h);
	hwfb->position(u0+w+u0, v0);
	hwfb->label("First Person View HW");
	hwfb->ishw = 1;
//	hwfb->show();
	hwfb->redraw();

	gui = new GUI();
	gui->show();
	gui->uiw->position(u0, v0 + fb->h + v0);

	tmsN = 2;
	tms = new TM[tmsN];
	tms[0].LoadBin("geometry/teapot57K.bin");
//	tms[0].LoadBin("geometry/teapot57K.bin");
	tms[0].Position(V3(0.0f, 0.0f, -150.0f));

	V3 vs[4];
	AABB aabb = tms[0].GetAABB();
	V3 qc = tms[0].GetCenter(); qc[1] = aabb.c0[1];
	float qs2 = 55.0f;
	vs[0] = qc + V3(-qs2, 0.0f, -qs2);
	vs[1] = qc + V3(-qs2, 0.0f, +qs2);
	vs[2] = qc + V3(+qs2, 0.0f, +qs2);
	vs[3] = qc + V3(+qs2, 0.0f, -qs2);
	tms[1].SetQuad(vs);
	tms[1].colors[0] =
		tms[1].colors[1] =
		tms[1].colors[2] =
		tms[1].colors[3] = V3(1.0f, 0.0f, 0.0f);
	FrameBuffer *texfb = new FrameBuffer(100, 100, 64, 64);
	texfb->SetChecker(8, 0xFF000000, 0xFFFFFFFF);
	tms[1].tex = texfb;

	// create camera
	float hfov = 60.0f;
	ppc = new PPC(hfov, fb->w, fb->h);
	ppc3 = new PPC(hfov, fb3->w, fb3->h);
	V3 LAP = (ppc->C + tms[0].GetCenter())*0.5f;
	V3 newC = ppc->C + V3(100.0f, 100.0f, 100.0f);
	V3 upv(0.0f, 1.0f, 0.0f);
	ppc3->Pose(newC, LAP, upv);

	// shaders
	cgi = 0;
	soi = 0;
	shadowShader = 0;
	projTexShader = 0;
	hwRenderMode = 0;
	shadowFBOHW = 0;
	shadowDepthTexHW = 0;
	shadowMapSize = 1024;
	projTexHW = 0;
	projectorPPC = 0;
	hwLightPPC = 0;
	hwLightPos = V3(40.0f, 60.0f, -50.0f);

	// scene resources
	envMap = 0;
	shadowFB = 0;
	lightPPC = 0;

	tms[0].on = 1;
	Render();

}

void Scene::Render(PPC *usePPC, FrameBuffer *useFB) {


	useFB->Set(0xFFFFFFFF);
	useFB->SetZB(0.0f); // 1/w = 0.0f means infinitely far away

	for (int tmi = 0; tmi < tmsN; tmi++) {
//		tms[tmi].DrawWireFrame(0xFF000000, usePPC, useFB);
		tms[tmi].RenderFilled(usePPC, useFB);
	}
	useFB->redraw();


}

void Scene::Render() {

	if (fb) {
		RayTrace();
		fb->redraw();
		return;
	}

	if (fb)
		Render(ppc, fb);
	if (hwfb)
		hwfb->redraw();
	if (fb3) {
		Render(ppc3, fb3);
	ppc->Visualize(ppc3, fb3, 15.0f);

#if 0
		fb3->Set(0xFFFFFFFF);
		fb3->SetZB(0.0f);
		fb3->DrawFBPointCloud(fb, ppc, ppc3);
		fb3->redraw();
#endif
	}

}

void Scene::DBG() {

	cerr << endl;

	{
		int fn = 1000;
		for (int fi = 0; fi < fn; fi++) {
			mFraction = (float)fi / (float)(fn - 1);
			Render();
			Fl::check();
		}
		return;
	}


	V3 C1 = ppc->C + V3(30.f, 100.0f, 0.0f);
	V3 C0 = ppc->C;
	V3 cn = tms[0].GetCenter();
	int fN = 100;
	for (int fi = 0; fi < fN; fi++) {
		Render();
		Fl::check();
		V3 C = C0 + (C1 - C0)*(float)fi / (float) (fN - 1);
		ppc->Pose(C, cn, V3(0.0f, 1.0f, 0.0f));
	}
	return;



	{


		V3 vs[4];
		vs[0] = V3(-40.0f, +20.0f, -100.0f);
		vs[1] = V3(-40.0f, -20.0f, -100.0f);
		vs[2] = V3(+40.0f, -20.0f, -100.0f);
		vs[3] = V3(+40.0f, +20.0f, -100.0f);

		vs[0] = V3(-40.0f, +40.0f, -10.0f);
		vs[1] = V3(-40.0f, -40.0f, -10.0f);
		vs[2] = V3(+40.0f, -40.0f, -10.0f);
		vs[3] = V3(+40.0f, +40.0f, -10.0f);

		tms[0].SetQuad(vs);

		fb3->hide();

		tms[0].tex = new FrameBuffer(10, 10, 1024, 1024);
		tms[0].tex->SetChecker(1, 0xFF000000, 0xFFFFFFFF);
//		tms[0].tex->show();
		for (int fi = 0; fi < 10000; fi++) {
			Render();
			Fl::check();
			tms[0].Translate(V3(0.0f, 0.0f, -.1f));
		}
		return;

		for (int i = 0; i < 45; i++) {
			Render();
			Fl::check();
			tms[0].RotateAboutArbitraryAxis(tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f), 1.0f);
		}
		return;
	}

	{
		for (int fi = 0; fi < 360; fi++) {
			Render();
			ppc->Pan(1.0f);
			Fl::check();
		}
		return;
	}

	{
		V3 P(0.0f, -20.0f, -100.0f);
		V3 n(0.0f, 1.0f, 0.0f);
		V3 l(-1.0f, 1.0f, 0.0f);
		V3 r = n.Reflect(l);
		fb->Set(0xFFFFFFFF);
		fb->SetZB(0.0f);
		fb->DrawPoint3D(P, ppc, 7, 0xFF000000);
		fb->Draw3DSegment(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f),
			ppc, P, P + l.Normalized()*17.0f);
		fb->Draw3DSegment(V3(0.0f, 0.0f, 0.0f), V3(1.0f, 0.0f, 0.0f),
			ppc, P, P + r.Normalized()*17.0f);
		fb->Draw3DSegment(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 0.0f, 1.0f),
			ppc, P, P + n.Normalized()*7.0f);
		fb->redraw();
		return;
	}


	{
		V3 ld(0.0f, 0.0f, 1.0f);
		float ka = 0.04f;
		V3 teapotCenter = tms[0].GetCenter();
		V3 L = teapotCenter + V3(0.0f, 0.0f, 30.0f);
		for (int fi = 0; fi < 360; fi++) {
			fb->Set(0xFFFFFFFF);
			fb->SetZB(0.0f);
			tms[0].LightP(L, ka);
			Render();
			fb->DrawPoint3D(L, ppc, 13, 0xFF00FFFF);
			fb->redraw();
			Fl::check();
			L = L.RotateThisPointAboutArbitraryAxis(teapotCenter, 
				V3(0.0f, 1.0f, 0.0f),
				1.0f);
//			L = L + V3(0.0f, 0.0f, -.1f);
		}
		return;
		tms[0].VisualizeNormals(9.0f, ppc, fb);
		fb->redraw();
		return;
	}

	{
		// load triangle mesh
		TM tm;
		tm.LoadBin("geometry/teapot57K.bin");

		tm.Position(V3(0.0f, 0.0f, -150.0f));

		// create camera
		float hfov = 60.0f;
		PPC ppc(hfov, fb->w, fb->h);
		// translate backwards to teapot
//		ppc.Translate(V3(0.0f, 0.0f, 250.0f));
		// clear framebuffer
		fb->Set(0xFFFFFFFF);

		// render mesh with one point per vertex; // no triangles (yet)
		tm.DrawPoints(0xFF000000, 3, &ppc, fb);

		// refresh window
		fb->redraw();

//		return;
		
		// spin the teapot in place (about a vertical axis passing through its center)
		V3 ad(0.0f, 1.0f, 0.0f);
		V3 aO = tm.GetCenter();
		for (int fi = 0; fi < 360; fi++) {
			fb->Set(0xFFFFFFFF);
//			tm.DrawPoints(0xFF000000, 3, &ppc, fb);
			tm.DrawWireFrame(0xFF000000, &ppc, fb);
			fb->redraw();
			Fl::check();
			tm.RotateAboutArbitraryAxis(aO, ad, 1.0f);
		}

		return;



	}


	{
		V3 p0(10.0f, 200.0f, 0.0f);
		V3 p1(210.0f, 200.0f, 0.0f);
		fb->Set(0xFFFFFFFF);
		V3 C0(1.0f, 0.0f, 0.0f);
		V3 C1(0.0f, 1.0f, 0.0f);
		fb->Draw2DSegment(C0, C1, p0, p1);
		fb->redraw();
		return;
	}


	{
		float hfov = 60.0f;
		PPC ppc(hfov, fb->w, fb->h);
		V3 PL(-5.0f, -5.0f, -20.0f);
		V3 PR(+5.0f, -5.0f, -20.0f);
		V3 PP;

		V3 cube[8];
		V3 cubeCenter(0.0f, 0.0f, -10.0f);
		float a = 2.0f;
		cube[0] = cubeCenter + V3(-a, a, a);
		cube[1] = cube[0] + V3(0.0f, -2.0f*a, 0.0f);
		cube[2] = cube[1] + V3(2.0f*a, 0.0f, 0.0f);
		cube[3] = cube[2] + V3(0.0f, 2.0f*a, 0.0f);

		int framesN = 3000;
		V3 tv(0.001f, 0.0f, 0.0f);
		for (int fi = 0; fi < framesN; fi++) {

			for (int vi = 0; vi < 4; vi++) {
				cube[vi + 4] = cube[vi] + V3(0, 0, -2.0f*a);
			}
			fb->Set(0xFFFFFFFF);
			for (int vi = 0; vi < 8; vi++) {
				V3 PP;
				ppc.Project(cube[vi], PP);
				if (vi < 4)
					fb->DrawPoint2D(PP, 13, 0xFF0000FF);
				else
					fb->DrawPoint2D(PP, 13, 0xFFFF0000);
			}
			fb->redraw();
			Fl::check();
			ppc.Translate(tv);
		}

		return;

		framesN = 1000;
		float tstep = 0.01f;
		for (int fi = 0; fi < framesN; fi++) {
			fb->Set(0xFFFFFFFF);
			ppc.Project(PL, PP);
			fb->DrawPoint2D(PP, 13, 0xFFFF0000);
			ppc.Project(PR, PP);
			fb->DrawPoint2D(PP, 13, 0xFFFF0000);
			fb->redraw();
			Fl::check();
			PL[2] -= tstep;
			PR[2] -= tstep;
		}

		return;

	}

	{
		V3 v0(10.0f, -1.0f, 0.0f);
		V3 v1(3.0f, 1.0f, 2.0f);
		V3 v2(-7.0f, 0.0f, 1.0f);
		M33 m(v0, v1, v2), minv;
		minv = m.Inverted();
		cerr << minv*m.GetColumn(0) << endl;
		cerr << minv*m.GetColumn(1) << endl;
		cerr << minv*m.GetColumn(2) << endl;
		return;


		cerr << m[0][0] << endl;
		m[0][0] = -1.0f;
		cerr << m[0][0] << endl;
		cerr << m*v2 << endl;
		return;
	}

	{
		V3 v(1.0f, 2.0f, 3.0f);
		cerr << "vector v: " << v << endl;
		cerr << "length of v: " << v.Length() << endl;
		cerr << "vx3: " << v*3.0f << endl;
		cerr << "v.Normalized(): " << v.Normalized() << endl;
		cerr << v.Normalized().Length() << endl;
		return;
	}

	int stepsN = 10000;
	for (int si = 0; si < stepsN; si++) {
		fb->Set(0xFFFFFFFF);
		int v = fb->h / 2;
		for (int u = 100; u < 200; u++) {
			fb->SetSafe(u + si, v, 0xFF000000);
		}
		fb->redraw();
		Fl::check();
	}
	return;


	V3 v0(1.0f, 2.0f, 3.0f);
	V3 v1(2.0f, 4.0f, 6.0f);
	V3 v;
	v = v0 + v1;
	return;


	fb->SetChecker(32, 0xFF000000, 0xFFFFFFFF);
	fb->redraw();
	return;


	fb->Set(0xFF00FF00);
	fb->redraw();
	return;

	for (int i = 0; i < fb->w*fb->h; i++) {
		fb->pix[i] = 0xFF0000FF;
	}
	
	fb->redraw();

	cerr << endl;
	cerr << "INFO: pressed DBG button on GUI" << endl;

}

void Scene::NewButton() {
	cerr << "INFO: pressed New button on GUI" << endl;
}



void Scene::RenderHW() {

	switch (hwRenderMode) {
	case 1:
		RenderHWShadowMap();
		return;
	case 2:
		RenderHWProjTex();
		return;
	default:
		break;
	}

	// default: basic HW rendering
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ppc->SetHWInstrinsics();
	ppc->SetHWExtrinsics();

	for (int tmi = 0; tmi < tmsN; tmi++) {
		int useShaders = (tmi == 0);
		if (useShaders) {
			cgi->EnableProfiles();
			soi->PerFrameInit();
			soi->BindPrograms();
		}
		tms[tmi].RenderHW();
		if (useShaders) {
			cgi->DisableProfiles();
			soi->PerFrameDisable();
		}
	}
}

void Scene::InitHWRendering() {

	cerr << "InitHWRendering" << endl;
	glEnable(GL_DEPTH_TEST);

	// Cg shaders (stubbed on macOS)
	cgi = new CGInterface();
	cgi->PerSessionInit();
	soi = new ShaderOneInterface();
	soi->PerSessionInit(cgi);

	// GLSL shaders
	shadowShader = new GLSLShader();
	shadowShader->Load("shaders/shadow.vert", "shaders/shadow.frag");
	projTexShader = new GLSLShader();
	projTexShader->Load("shaders/projTex.vert", "shaders/projTex.frag");

	// Shadow map FBO
	SetupShadowFBO();

	// Projective texture
	SetupProjTexture();

	// Light camera for shadow mapping
	hwLightPPC = new PPC(90.0f, shadowMapSize, shadowMapSize);
	V3 sceneCenter = tms[0].GetCenter();
	hwLightPPC->Pose(hwLightPos, sceneCenter, V3(0.0f, 0.0f, 1.0f));

	// Projector camera
	projectorPPC = new PPC(60.0f, 512, 512);
	V3 projPos(60.0f, 40.0f, -80.0f);
	projectorPPC->Pose(projPos, sceneCenter, V3(0.0f, 1.0f, 0.0f));
}

void Scene::SetupShadowFBO() {
	// depth texture
	glGenTextures(1, &shadowDepthTexHW);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexHW);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize, shadowMapSize,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	// FBO
	glGenFramebuffers(1, &shadowFBOHW);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBOHW);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexHW, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		cerr << "ERROR: shadow FBO incomplete: " << status << endl;
	else
		cerr << "INFO: shadow FBO created " << shadowMapSize << "x" << shadowMapSize << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Scene::SetupProjTexture() {
	// create a checker texture as the projected image
	int texW = 256, texH = 256;
	unsigned char *data = new unsigned char[texW * texH * 3];
	int cw = 32;
	for (int v = 0; v < texH; v++) {
		for (int u = 0; u < texW; u++) {
			int checker = ((u / cw) + (v / cw)) % 2;
			// make it colorful: red/cyan checker
			data[(v * texW + u) * 3 + 0] = checker ? 255 : 0;
			data[(v * texW + u) * 3 + 1] = checker ? 0 : 255;
			data[(v * texW + u) * 3 + 2] = checker ? 0 : 255;
		}
	}
	glGenTextures(1, &projTexHW);
	glBindTexture(GL_TEXTURE_2D, projTexHW);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texW, texH, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	delete[] data;
	cerr << "INFO: projective texture created" << endl;
}

void Scene::GetMVPMatrix(PPC *cam, float *mvp) {
	// Save current matrices, set camera matrices, extract MVP, restore
	float proj[16], modelview[16];

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	cam->SetHWInstrinsics();
	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	cam->SetHWExtrinsics();
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
	glPopMatrix();

	// multiply proj * modelview (column-major)
	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			mvp[c * 4 + r] = 0.0f;
			for (int k = 0; k < 4; k++) {
				mvp[c * 4 + r] += proj[k * 4 + r] * modelview[c * 4 + k];
			}
		}
	}
}

void Scene::RenderHWShadowMap() {
	// Pass 1: render depth from light's perspective into shadow FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBOHW);
	glViewport(0, 0, shadowMapSize, shadowMapSize);
	glClear(GL_DEPTH_BUFFER_BIT);

	hwLightPPC->SetHWInstrinsics();
	hwLightPPC->SetHWExtrinsics();

	for (int tmi = 0; tmi < tmsN; tmi++)
		tms[tmi].RenderHW();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Get light MVP matrix
	float lightMVP[16];
	GetMVPMatrix(hwLightPPC, lightMVP);

	// Pass 2: render scene from camera with shadow shader
	glViewport(0, 0, hwfb->w, hwfb->h);
	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ppc->SetHWInstrinsics();
	ppc->SetHWExtrinsics();

	shadowShader->Enable();
	shadowShader->SetUniformMatrix4("lightMVP", lightMVP);
	shadowShader->SetUniform("lightPos", hwLightPos);
	shadowShader->SetUniform("ka", 0.25f);
	shadowShader->SetUniform("shadowMap", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowDepthTexHW);

	for (int tmi = 0; tmi < tmsN; tmi++)
		tms[tmi].RenderHW();

	glBindTexture(GL_TEXTURE_2D, 0);
	shadowShader->Disable();
}

void Scene::RenderHWProjTex() {
	// Get projector MVP matrix
	float projMVP[16];
	GetMVPMatrix(projectorPPC, projMVP);

	// Render scene from camera with projective texture shader
	glViewport(0, 0, hwfb->w, hwfb->h);
	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ppc->SetHWInstrinsics();
	ppc->SetHWExtrinsics();

	projTexShader->Enable();
	projTexShader->SetUniformMatrix4("projectorMVP", projMVP);
	projTexShader->SetUniform("projectorPos", projectorPPC->C);
	projTexShader->SetUniform("ka", 0.25f);
	projTexShader->SetUniform("projTexture", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, projTexHW);

	for (int tmi = 0; tmi < tmsN; tmi++)
		tms[tmi].RenderHW();

	glBindTexture(GL_TEXTURE_2D, 0);
	projTexShader->Disable();
}

// Camera path
void Scene::SaveCameraKeyframe() {
	cameraPath.push_back(*ppc);
	cerr << "INFO: saved keyframe " << cameraPath.size() << endl;
}

void Scene::ClearCameraPath() {
	cameraPath.clear();
	cerr << "INFO: camera path cleared" << endl;
}

void Scene::PlayCameraPath() {
	if (cameraPath.size() < 2) {
		cerr << "INFO: need at least 2 keyframes" << endl;
		return;
	}

	int framesPerSegment = 90; // 3 seconds at 30fps per segment
	cerr << "INFO: playing camera path with " << cameraPath.size() << " keyframes" << endl;

	for (int seg = 0; seg < (int)cameraPath.size() - 1; seg++) {
		for (int fi = 0; fi < framesPerSegment; fi++) {
			float t = (float)fi / (float)framesPerSegment;
			PPC interpPPC = cameraPath[seg].InterpolateSmooth(cameraPath[seg + 1], t);
			*ppc = interpPPC;
			ppc->Update();

			if (hwfb->visible()) {
				hwfb->redraw();
			} else {
				Render();
			}
			Fl::check();
		}
	}
	cerr << "INFO: camera path playback done" << endl;
}


void Scene::RayTrace() {

	fb->redraw();

	for (int v = 0; v < fb->h; v++) {
		cerr << "INFO: " << (float)v / (float)fb->h << "           \r";
		for (int u = 0; u < fb->w; u++)
			fb->Set(u, v, 0xFF0000FF);
		fb->redraw();
		Fl::check();

		for (int u = 0; u < fb->w; u++) {
			V3 O = ppc->C;
			V3 ray = ppc->GetRay(u, v);
			float t;
			V3 dcolor, color;
			V3 rO, rray;
			if (IntersectWithRay(O, ray, dcolor, t, rO, rray)) {
				float eps = 0.001f;
				rO = rO + rray.Normalized()*eps;
				V3 rcolor;
				V3 rrO, rrray;
				if (IntersectWithRay(rO, rray, rcolor, t, rrO, rrray)) {
					color = (dcolor + rcolor)*0.5f;
				}
				else
					color = dcolor;
				fb->Set(u, v, color.GetColor());
			}
			else
				fb->Set(u, v, 0xFF808080);
		}
	}

}

int Scene::IntersectWithRay(V3 O, V3 ray, V3& color, float &t, V3 &rO, V3 &rray) {

	t = FLT_MAX;
	for (int tmi = 0; tmi < tmsN; tmi++) {
		if (!tms[tmi].on)
			continue;
		V3 ccolor, crO, crray;
		float ct;
		if (!tms[tmi].IntersectWithRay(O, ray, ccolor, ct, crO, crray))
			continue;
		if (ct > t)
			continue;
		t = ct;
		color = ccolor;
		rO = crO;
		rray = crray;
	}
	if (t == FLT_MAX)
		return 0;
	return 1;
}


// ---- Scene Rendering Techniques ----

void Scene::RenderMeshLighting() {
	V3 teapotCenter = tms[0].GetCenter();
	V3 lightPos = teapotCenter + V3(0.0f, 30.0f, 50.0f);
	float ka = 0.05f;
	float specularIntensity = 32.0f;

	int fn = 360;
	for (int fi = 0; fi < fn; fi++) {
		fb->Set(0xFF000000);
		fb->SetZB(0.0f);

		for (int tmi = 0; tmi < tmsN; tmi++) {
			tms[tmi].DrawFilledPointLight(ppc, fb, lightPos, ka, specularIntensity);
		}

		// draw light indicator
		fb->DrawPoint3D(lightPos, ppc, 7, 0xFFFFFFFF);

		fb->redraw();
		Fl::check();

		// orbit the light around the teapot
		lightPos = lightPos.RotateThisPointAboutArbitraryAxis(
			teapotCenter, V3(0.0f, 1.0f, 0.0f), 1.0f);
	}
}

void Scene::RenderShadowMap() {
	// Setup light camera if needed
	V3 teapotCenter = tms[0].GetCenter();
	V3 lightPos = teapotCenter + V3(0.0f, 50.0f, 60.0f);
	float ka = 0.05f;
	float specularIntensity = 32.0f;
	float shadowEps = 0.3f;

	int shadowRes = 256;
	if (!shadowFB) {
		shadowFB = new FrameBuffer(0, 0, shadowRes, shadowRes);
		shadowFB->hide();
	}
	if (!lightPPC) {
		lightPPC = new PPC(90.0f, shadowRes, shadowRes);
	}

	// Position light camera looking at the scene
	lightPPC->Pose(lightPos, teapotCenter, V3(0.0f, 1.0f, 0.0f));

	int fn = 360;
	for (int fi = 0; fi < fn; fi++) {
		// Pass 1: render from light's POV to get shadow depth map
		shadowFB->Set(0xFF000000);
		shadowFB->SetZB(0.0f);
		for (int tmi = 0; tmi < tmsN; tmi++) {
			tms[tmi].RenderFilled(lightPPC, shadowFB);
		}

		// Pass 2: render from camera with shadow test
		fb->Set(0xFF000000);
		fb->SetZB(0.0f);
		for (int tmi = 0; tmi < tmsN; tmi++) {
			tms[tmi].DrawFilledPointLightShadow(ppc, fb, lightPos, ka, specularIntensity,
				lightPPC, shadowFB, shadowEps);
		}

		// draw light indicator
		fb->DrawPoint3D(lightPos, ppc, 7, 0xFFFFFFFF);

		fb->redraw();
		Fl::check();

		// orbit light
		lightPos = lightPos.RotateThisPointAboutArbitraryAxis(
			teapotCenter, V3(0.0f, 1.0f, 0.0f), 1.0f);
		lightPPC->Pose(lightPos, teapotCenter, V3(0.0f, 1.0f, 0.0f));
	}
}

void Scene::RenderTextured() {
	fb->Set(0xFFFFFFFF);
	fb->SetZB(0.0f);

	// Render all meshes: textured if they have a texture, otherwise filled
	for (int tmi = 0; tmi < tmsN; tmi++) {
		if (tms[tmi].tex && tms[tmi].tcs) {
			tms[tmi].DrawTextured(ppc, fb, tms[tmi].tex, 1, 0); // bilinear, repeat
		} else {
			tms[tmi].RenderFilled(ppc, fb);
		}
	}

	fb->redraw();
}

void Scene::RenderEnvMap() {
	// Create cubemap if needed
	if (!envMap) {
		const char* sides[6] = {
			"geometry/uffizi_front.tiff",
			"geometry/uffizi_left.tiff",
			"geometry/uffizi_back.tiff",
			"geometry/uffizi_right.tiff",
			"geometry/uffizi_top.tiff",
			"geometry/uffizi_bottom.tiff"
		};
		envMap = new CubeMap(sides);
	}

	int fn = 360;
	for (int fi = 0; fi < fn; fi++) {
		fb->Set(0xFF000000);
		fb->SetZB(0.0f);

		// Render environment-mapped teapot
		tms[0].DrawFilledEnvMap(ppc, fb, envMap);

		// Render other meshes normally
		for (int tmi = 1; tmi < tmsN; tmi++) {
			tms[tmi].RenderFilled(ppc, fb);
		}

		fb->redraw();
		Fl::check();

		// slowly rotate the teapot
		tms[0].RotateAboutArbitraryAxis(tms[0].GetCenter(), V3(0.0f, 1.0f, 0.0f), 1.0f);
	}
}

void Scene::RenderSoftShadow() {
	// Save original scene
	TM *origTms = tms;
	int origTmsN = tmsN;

	// Create soft shadow scene: ground plane + 3 boxes
	tmsN = 4;
	tms = new TM[tmsN];

	// Ground plane (smaller to reduce pixel count)
	tms[0].LoadPlane(V3(0.0f, -15.0f, -100.0f), V3(100.0f, 0.0f, 80.0f), V3(0.6f, 0.6f, 0.6f));

	// Three boxes with different sizes and colors
	tms[1].LoadRectangleWithNormals(V3(-25.0f, -5.0f, -100.0f), V3(14.0f, 20.0f, 14.0f), V3(0.8f, 0.2f, 0.2f));
	tms[2].LoadRectangleWithNormals(V3(5.0f, -7.0f, -110.0f), V3(12.0f, 16.0f, 12.0f), V3(0.2f, 0.8f, 0.2f));
	tms[3].LoadRectangleWithNormals(V3(30.0f, -3.0f, -95.0f), V3(10.0f, 24.0f, 10.0f), V3(0.2f, 0.2f, 0.8f));

	// Soft shadow parameters
	int k = 4; // 4x4 = 16 light samples for smoother penumbra
	int numSamples = k * k;
	int shadowRes = 384;
	float ka = 0.25f;
	float specularIntensity = 0.0f; // disabled for performance
	float shadowEps = 0.002f;

	// Rectangular area light - larger for softer shadows
	V3 lightCenter(40.0f, 50.0f, -60.0f);
	V3 lightRight(1.0f, 0.0f, 0.0f);
	V3 lightUp(0.0f, 0.0f, 1.0f);
	float lightHalfW = 15.0f;
	float lightHalfH = 15.0f;
	V3 sceneCenter(0.0f, -5.0f, -100.0f);

	// Allocate k*k light cameras and shadow buffers
	PPC **lightPPCs = new PPC*[numSamples];
	FrameBuffer **shadowFBs = new FrameBuffer*[numSamples];
	for (int i = 0; i < numSamples; i++) {
		lightPPCs[i] = new PPC(90.0f, shadowRes, shadowRes);
		shadowFBs[i] = new FrameBuffer(0, 0, shadowRes, shadowRes);
		shadowFBs[i]->hide();
	}

	int fn = 180;
	for (int fi = 0; fi < fn; fi++) {
		// Animate: orbit light around scene
		float angle = (float)fi * 2.0f; // 360 degrees over 180 frames
		V3 animLightCenter = lightCenter.RotateThisPointAboutArbitraryAxis(
			sceneCenter, V3(0.0f, 1.0f, 0.0f), angle);
		V3 animRight = lightRight.RotateThisVectorAboutArbitraryAxis(V3(0.0f, 1.0f, 0.0f), angle);
		V3 animUp = lightUp.RotateThisVectorAboutArbitraryAxis(V3(0.0f, 1.0f, 0.0f), angle);

		// Boxes are static on the ground plane

		// Position k*k light sample cameras
		for (int j = 0; j < k; j++) {
			for (int i = 0; i < k; i++) {
				int idx = j * k + i;
				float u = -1.0f + 2.0f * (i + 0.5f) / k;
				float v = -1.0f + 2.0f * (j + 0.5f) / k;
				V3 samplePos = animLightCenter + animRight * (u * lightHalfW) + animUp * (v * lightHalfH);
				lightPPCs[idx]->Pose(samplePos, sceneCenter, V3(0.0f, 0.0f, 1.0f));
			}
		}

		// Pass 1: render shadow casters (boxes only, skip ground plane tms[0])
		for (int si = 0; si < numSamples; si++) {
			shadowFBs[si]->Set(0xFF000000);
			shadowFBs[si]->SetZB(0.0f);
			for (int tmi = 1; tmi < tmsN; tmi++) {
				tms[tmi].RenderFilled(lightPPCs[si], shadowFBs[si]);
			}
		}

		// Pass 2: render final image with soft shadows
		fb->Set(0xFF000000);
		fb->SetZB(0.0f);
		for (int tmi = 0; tmi < tmsN; tmi++) {
			tms[tmi].DrawFilledSoftShadow(ppc, fb, animLightCenter, ka, specularIntensity,
				lightPPCs, shadowFBs, numSamples, shadowEps);
		}

		// Draw light position indicator
		fb->DrawPoint3D(animLightCenter, ppc, 5, 0xFFFFFFFF);

		fb->redraw();
		Fl::check();
	}

	// Cleanup
	for (int i = 0; i < numSamples; i++) {
		delete lightPPCs[i];
		delete shadowFBs[i];
	}
	delete[] lightPPCs;
	delete[] shadowFBs;

	// Restore original scene
	delete[] tms;
	tms = origTms;
	tmsN = origTmsN;
}