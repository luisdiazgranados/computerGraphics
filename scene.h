#pragma once
#include "gui.h"
#include "framebuffer.h"

class Scene {
public:

	GUI *gui;
	FrameBuffer *fb;
	Scene();
	void DBG();
	void NewButton();
	void drawLine(int x0, int y0, int x1, int y1, unsigned int color);
	void drawCircle(int cx, int cy, int r, unsigned int color);
	void drawRectangle(int x0, int y0, int x1, int y1, unsigned int color);
	void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, unsigned int color);
	void drawL(int x, int y, int w, int h, int t, unsigned int color);
	void drawU(int x, int y, int w, int h, int t, unsigned int color);
	void drawI(int x, int y, int w, int h, int t, unsigned int color);
	int mapX(float deg, int L, int W, float degstart = 0.0f, float degend = 360.0f);
	int mapY(float v, float vmin, float vmax, int T, int B, int H);
	void plotHW2(FrameBuffer* fb);
};

extern Scene *scene;