#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int *pix;
	int w, h;
	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	int handle(int guievent);
	void LoadTiff(char* fname);
	void SaveAsTiff(char* fname);
	void KeyboardHandle();

	void Set(unsigned int color);
	void Set(unsigned int u, unsigned int v, unsigned int color);
};