//
// Shinjiro Sueda
// October, 2014
//
// Adapted from:
//   GLSL_helper.h
//   CSC473
//
//    Many useful helper functions for GLSL shaders - gleaned from various sources including orange book
//    Created by zwood on 2/21/10.

#ifdef __APPLE__
    #define GLFW_INCLUDE_GLCOREARB
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#pragma once
#ifndef __GLSL__
#define __GLSL__

namespace GLSL {
	
	int printError(const char *file, int line);
	void printProgramInfoLog(GLuint program);
	void printShaderInfoLog(GLuint shader);
	GLint getUniLoc(GLuint program, const GLchar *name);
	void checkVersion();
	int textFileWrite(const char *filename, char *s);
	char *textFileRead(const char *filename);
	GLint getAttribLocation(const GLuint program, const char varname[]);
	GLint getUniformLocation(const GLuint program, const char varname[]);
	void enableVertexAttribArray(const GLint handle);
	void disableVertexAttribArray(const GLint handle);
	void vertexAttribPointer(const GLint handle, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
}

#endif
