#pragma once

#include "v3.h"

// Color byte layout: 0xAABBGGRR (matches GL_RGBA on little-endian)

inline unsigned int ColorFromRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return (a << 24) | (b << 16) | (g << 8) | r;
}

inline unsigned int ColorFromRGB(unsigned char r, unsigned char g, unsigned char b) {
	return (0xFF << 24) | (b << 16) | (g << 8) | r;
}

inline unsigned int ColorFromV3(V3 v) {
	return ColorFromRGB(
		(unsigned char)(v[0] * 255.0f),
		(unsigned char)(v[1] * 255.0f),
		(unsigned char)(v[2] * 255.0f));
}

inline V3 V3FromColor(unsigned int c) {
	unsigned char r = (c >> 0) & 0xFF;
	unsigned char g = (c >> 8) & 0xFF;
	unsigned char b = (c >> 16) & 0xFF;
	return V3(r / 255.0f, g / 255.0f, b / 255.0f);
}

inline unsigned int ColorFromInverseZ(float invZ) {
	float scale = 0.1f;
	float z = 1.0f - (1.0f / (1.0f + invZ * scale));
	unsigned char i = (unsigned char)(z * 255.0f);
	return ColorFromRGB(i, i, i);
}

inline unsigned int ColorAlpha(unsigned int c) { return (c >> 24) & 0xFF; }
inline unsigned int ColorRed(unsigned int c)   { return (c >> 0) & 0xFF; }
inline unsigned int ColorGreen(unsigned int c) { return (c >> 8) & 0xFF; }
inline unsigned int ColorBlue(unsigned int c)  { return (c >> 16) & 0xFF; }

inline unsigned int ColorBlend4(unsigned int a, unsigned int b, unsigned int c, unsigned int d) {
	unsigned int red   = (ColorRed(a) + ColorRed(b) + ColorRed(c) + ColorRed(d)) / 4;
	unsigned int green = (ColorGreen(a) + ColorGreen(b) + ColorGreen(c) + ColorGreen(d)) / 4;
	unsigned int blue  = (ColorBlue(a) + ColorBlue(b) + ColorBlue(c) + ColorBlue(d)) / 4;
	unsigned int alpha = (ColorAlpha(a) + ColorAlpha(b) + ColorAlpha(c) + ColorAlpha(d)) / 4;
	return ColorFromRGBA(red, green, blue, alpha);
}
