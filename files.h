#ifndef _files_h_
#define _files_h_

#include <GL/glew.h>
#include <GL/freeglut.h>

extern char* readTextFile(const char* filename);
extern char* readPngFile(const char* filename, int *width, int *height, GLenum *format);

#endif
