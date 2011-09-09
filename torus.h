#ifndef _torus_h_
#define _torus_h_

#include <GL/glew.h>

GLuint createTorusPositions(int n, float r, float R);
inline int torusAttributeCount(int n) { return n * n * 6;}

#endif
