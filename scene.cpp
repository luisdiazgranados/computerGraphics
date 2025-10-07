#include "GL/glew.h"

#include "scene.h"
#include "v3.h"
#include "M33.h"
#include <algorithm>

Scene *scene;


#include <iostream>
#include <fstream>
#include <strstream>

using namespace std;

void Scene::drawLine(int x0, int y0, int x1, int y1, unsigned int color) {
	int sx;
	int sy;

	//deltas 
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	//step direction
	if (x0 < x1) sx = 1;
	else sx = -1;
	if (y0 < y1) sy = 1;
	else sy = -1;

	int error = 0;
	int u, v;

	if (dx >= dy) { //x major axis
		v = y0;
		for (u = x0; u != x1; u += sx) {
			fb->Set(u, v, color);

			error += dy;
			if (error >= dx) { //increment y axis by sy when certain num of pixels are drawn
				v += sy;
				error -= dx;
			}
		}
		fb->Set(u, y1, color); //draw last pixel
	}
	else { //y major axis
		u = x0;
		for (v = y0; v != y1; v += sy) {
			fb->Set(u, v, color);

			error += dx;
			if (error >= dy) { //increment x axis by sx when certain num of pixels are drawn
				u += sx;
				error -= dy;
			}
		}
		fb->Set(x1, v, color); //draw last pixel
	}
}

void Scene::drawCircle(int cx, int cy, int r, unsigned int color) {
	int x = r;
	int y = 0;
	int decision = 1 - r;

	while (y <= x) {
		fb->Set(cx + x, cy + y, color);
		fb->Set(cx - x, cy + y, color);
		fb->Set(cx + x, cy - y, color);
		fb->Set(cx - x, cy - y, color);
		fb->Set(cx + y, cy + x, color);
		fb->Set(cx - y, cy + x, color);
		fb->Set(cx + y, cy - x, color);
		fb->Set(cx - y, cy - x, color);

		if (decision < 0) { //if midpoint is inside circle, increment y
			y++;
			decision += (2 * y) + 1;
		}
		else { //else midpoint is outside circle, increment x
			x--;
			y++;
			decision += 2 * (y - x) + 1;
		}
	}
}

void Scene::drawRectangle(int x0, int y0, int x1, int y1, unsigned int color) {
	//normalized variables
	int x_min = min(x0, x1);
	int x_max = max(x0, x1);
	int y_min = min(y0, y1);
	int y_max = max(y0, y1);
	int w = x_max - x_min;
	int h = y_max - y_min;

	if (h == 0) {
		drawLine(x_min, y_min, x_max, y_min, color); //horizontal line
	}
	else if (w == 0) {
		drawLine(x_min, y_min, x_min, y_max, color); //vertical line
	}

	else {
		drawLine(x_min, y_min, x_max, y_min, color); //top horizontal line
		drawLine(x_max, y_min, x_max, y_max, color); //right vertical line
		drawLine(x_max, y_max, x_min, y_max, color); //bottom horizontal line
		drawLine(x_min, y_max, x_min, y_min, color); //left vertical line
	}
}

void Scene::drawL(int x, int y, int w, int h, int t, unsigned int color) {
	drawRectangle(x, y, (x + t - 1), (y + h - 1), color);
	drawRectangle(x, (y + h - t), (x + w - 1), (y + h - 1), color);
}

void Scene::drawU(int x, int y, int w, int h, int t, unsigned int color) {
	drawRectangle(x, y, (x + t - 1), (y + h - 1), color);
	drawRectangle((x + w - t), y, (x + w - 1), (y + h - 1), color);
	drawRectangle((x + t), (y + h - t), (x + w - t - 1), (y + h - 1), color);
}

void Scene::drawI(int x, int y, int w, int h, int t, unsigned int color) {
	int xs = (x + (w - t) / 2);

	drawRectangle(xs, y, (xs + t - 1), (y + h - 1), color);
	drawRectangle(x, y, (x + w - 1), (y + t - 1), color);
	drawRectangle(x, (y + h - t), (x + w - 1), (y + h - 1), color);
}

void Scene::drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, unsigned int color) {
	drawLine(x0, y0, x1, y1, color);
	drawLine(x1, y1, x2, y2, color);
	drawLine(x2, y2, x0, y0, color);
}

int Scene::mapX(float deg, int L, int W, float degstart, float degend) {
	float t = (deg - float(degstart)) / float(degend - degstart);
	if (t < 0) t = 0;
	if (t > 1 ) t = 1;
	return L + int(t * float(W - 1) + 0.5f);
}

int Scene::mapY(float v, float vmin, float vmax, int T, int B, int H) {
    const float epsilon = 1e-6f;
    if (fabs(vmax - vmin) < epsilon) {
        return T + H / 2;
    }

    float t = (v - vmin) / (vmax - vmin);
    if (t < 0) t = 0;
    if (t > 1) t = 1;

    return T + int((1.0f - t) * float(H - 1) + 0.5f);
}
void Scene::plotHW2(FrameBuffer* fb) {
    V3 P0(1.5f, 0.5f, 2.0f);
    V3 Oa(0.0f, 0.0f, 0.0f);
    V3 a(0.0f, 1.0f, 0.0f);

    static const float degstart = 0.0f;
    static const float degend = 720.0f; //360 degrees 2 times so 720
    static const float step = 2.0f;

    static const int N = int((degend - degstart) / step) + 1; //361

    cerr << "Number of samples: " << N << endl;

    float angles[361]; //passing the N var was giving me issues
    float xs[361], ys[361], zs[361]; //same here

    for (int i = 0; i < N; ++i) {
        float th = degstart + i * step;
        V3 Pi = P0;
        Pi.rotateAxis(Oa, a, th);

        angles[i] = th;
        xs[i] = Pi[0];
        ys[i] = Pi[1];
        zs[i] = Pi[2];

        if (i > 0) {
            float dx = fabs(xs[i] - xs[i - 1]);
            float dy = fabs(ys[i] - ys[i - 1]);
            float dz = fabs(zs[i] - zs[i - 1]);

            if (dx > 0.5f || dy > 0.5f || dz > 0.5f) { //debug code
                cerr << "large jump at angle " << th;
                cerr << "dx=" << dx << " dy=" << dy << " dz=" << dz << endl;
                cerr << "prev: x=" << xs[i - 1] << " y=" << ys[i - 1] << " z=" << zs[i - 1] << endl;
                cerr << "curr:  x=" << xs[i] << " y=" << ys[i] << " z=" << zs[i] << endl;
            }
        }
    }

    float xmin = xs[0], xmax = xs[0];
    float ymin = ys[0], ymax = ys[0];
    float zmin = zs[0], zmax = zs[0];

    for (int i = 1; i < N; ++i) {
        if (xs[i] < xmin) xmin = xs[i]; if (xs[i] > xmax) xmax = xs[i];
        if (ys[i] < ymin) ymin = ys[i]; if (ys[i] > ymax) ymax = ys[i];
        if (zs[i] < zmin) zmin = zs[i]; if (zs[i] > zmax) zmax = zs[i];
    }

    cerr << "X range: [" << xmin << ", " << xmax << "]" << endl;
    cerr << "Y range: [" << ymin << ", " << ymax << "]" << endl;
    cerr << "Z range: [" << zmin << ", " << zmax << "]" << endl;

    fb->Set(0xFFFFFFFF);
    int L = 50, T = 40, R = fb->w - 30, B = fb->h - 60;
    int W = R - L, H = B - T;

    for (int u = L; u <= R; ++u) fb->Set(u, B, 0xFF000000); //x-axis
    for (int v = T; v <= B; ++v) fb->Set(L, v, 0xFF000000); //y-axis


	//was getting spikes in my graph, so i added this debug code to find them
	//it plots the points individually so I could see where the spikes were
	/*
    for (int i = 0; i < N; ++i) {
        int x = mapX(angles[i], L, W, degstart, degend);
        int yx = mapY(xs[i], xmin, xmax, T, B, H);
        int yy = mapY(ys[i], ymin, ymax, T, B, H);
        int yz = mapY(zs[i], zmin, zmax, T, B, H);

        if (x > L && x < R - 1 && yx > T && yx < B - 1) {
            fb->Set(x - 1, yx, 0xFFFF0000);
            fb->Set(x, yx, 0xFFFF0000);
            fb->Set(x + 1, yx, 0xFFFF0000);
            fb->Set(x, yx - 1, 0xFFFF0000);
            fb->Set(x, yx + 1, 0xFFFF0000);
        }

        if (x > L && x < R - 1 && yy > T && yy < B - 1) {
            fb->Set(x - 1, yy, 0xFF00AA00);
            fb->Set(x, yy, 0xFF00AA00);
            fb->Set(x + 1, yy, 0xFF00AA00);
            fb->Set(x, yy - 1, 0xFF00AA00);
            fb->Set(x, yy + 1, 0xFF00AA00);
        }

        if (x > L && x < R - 1 && yz > T && yz < B - 1) {
            fb->Set(x - 1, yz, 0xFF0033CC);
            fb->Set(x, yz, 0xFF0033CC);
            fb->Set(x + 1, yz, 0xFF0033CC);
            fb->Set(x, yz - 1, 0xFF0033CC);
            fb->Set(x, yz + 1, 0xFF0033CC);
        }

        if (i > 0) {
            int prevX = mapX(angles[i - 1], L, W, degstart, degend);
            int prevYy = mapY(ys[i - 1], ymin, ymax, T, B, H);

            if (abs(x - prevX) <= 2 && abs(yy - prevYy) > 50) {
                cerr << "spike at" << angles[i];
                cerr << "x=" << x << " prevX=" << prevX << " yy=" << yy << " prevYy=" << prevYy << endl;
            }
        }
    }
	*/

    for (int i = 1; i < N; ++i) {
        int x0 = mapX(angles[i-1], L, W, degstart, degend);
        int x1 = mapX(angles[i], L, W, degstart, degend);

        int yx0 = mapY(xs[i-1], xmin, xmax, T, B, H);
        int yx1 = mapY(xs[i], xmin, xmax, T, B, H);

        int yy0 = mapY(ys[i-1], ymin, ymax, T, B, H);
        int yy1 = mapY(ys[i], ymin, ymax, T, B, H);

        int yz0 = mapY(zs[i-1], zmin, zmax, T, B, H);
        int yz1 = mapY(zs[i], zmin, zmax, T, B, H);

        drawLine(x0, yx0, x1, yx1, 0xFFFF0000);
        drawLine(x0, yy0, x1, yy1, 0xFF00FF00);
        drawLine(x0, yz0, x1, yz1, 0xFF0000FF);
    }

    fb->redraw();
    fb->SaveAsTiff("HW2_plot.tiff");
}


Scene::Scene() {


	int u0 = 20;
	int v0 = 40;
	int h = 400;
	int w = 600;
	fb = new FrameBuffer(u0, v0, w, h);
	fb->position(u0, v0);
	fb->label("SW Framebuffer");
	fb->show();
	fb->redraw();

	gui = new GUI();
	gui->show();
	gui->uiw->position(u0, v0 + fb->h + v0);

}


void Scene::DBG() {

	plotHW2(fb);

	return;
	
	fb->redraw();

	cerr << endl;
	cerr << "INFO: pressed DBG button on GUI" << endl;

}

void Scene::NewButton() {
	cerr << "INFO: pressed New button on GUI" << endl;
}

