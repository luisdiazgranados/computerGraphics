#pragma once

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include "v3.h"

class GLSLShader {
	GLuint program;
	GLuint vertShader, fragShader;
	GLuint CompileShader(const char *path, GLenum type);
public:
	GLSLShader();
	bool Load(const char *vertPath, const char *fragPath);
	void Enable();
	void Disable();
	GLint GetUniformLocation(const char *name);
	void SetUniform(const char *name, float f);
	void SetUniform(const char *name, int i);
	void SetUniform(const char *name, V3 v);
	void SetUniformMatrix4(const char *name, float *m);
	GLuint GetProgram() { return program; }
};
