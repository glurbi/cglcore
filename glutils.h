#ifndef _glutils_h_
#define _glutils_h_

#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h>
#endif
#include <GL/freeglut.h>

extern void checkShaderCompileStatus(GLuint shaderId);
extern void checkProgramLinkStatus(GLuint programId);
extern void setSwapInterval(int interval);

#endif
