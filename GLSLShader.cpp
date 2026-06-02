#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include "GLSLShader.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

GLSLShader::GLSLShader() : program(0), vertShader(0), fragShader(0) {}

GLuint GLSLShader::CompileShader(const char *path, GLenum type) {
	ifstream file(path);
	if (!file.is_open()) {
		cerr << "ERROR: cannot open shader file: " << path << endl;
		return 0;
	}
	stringstream ss;
	ss << file.rdbuf();
	string src = ss.str();
	const char *csrc = src.c_str();

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &csrc, NULL);
	glCompileShader(shader);

	GLint ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		char log[1024];
		glGetShaderInfoLog(shader, sizeof(log), NULL, log);
		cerr << "ERROR compiling " << path << ":" << endl << log << endl;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

bool GLSLShader::Load(const char *vertPath, const char *fragPath) {
	vertShader = CompileShader(vertPath, GL_VERTEX_SHADER);
	if (!vertShader) return false;
	fragShader = CompileShader(fragPath, GL_FRAGMENT_SHADER);
	if (!fragShader) return false;

	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	GLint ok;
	glGetProgramiv(program, GL_LINK_STATUS, &ok);
	if (!ok) {
		char log[1024];
		glGetProgramInfoLog(program, sizeof(log), NULL, log);
		cerr << "ERROR linking shader program:" << endl << log << endl;
		return false;
	}
	cerr << "INFO: loaded shaders " << vertPath << " + " << fragPath << endl;
	return true;
}

void GLSLShader::Enable() { glUseProgram(program); }
void GLSLShader::Disable() { glUseProgram(0); }

GLint GLSLShader::GetUniformLocation(const char *name) {
	return glGetUniformLocation(program, name);
}

void GLSLShader::SetUniform(const char *name, float f) {
	glUniform1f(GetUniformLocation(name), f);
}

void GLSLShader::SetUniform(const char *name, int i) {
	glUniform1i(GetUniformLocation(name), i);
}

void GLSLShader::SetUniform(const char *name, V3 v) {
	glUniform3f(GetUniformLocation(name), v[0], v[1], v[2]);
}

void GLSLShader::SetUniformMatrix4(const char *name, float *m) {
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, m);
}
