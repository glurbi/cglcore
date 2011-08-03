#ifndef _glutils_h_
#define _glutils_h_

#include <GL/glew.h>
#include <GL/freeglut.h>

extern void checkShaderCompileStatus(GLuint shaderId);
extern void checkProgramLinkStatus(GLuint programId);

#endif
