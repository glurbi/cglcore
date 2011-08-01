#ifndef _matrices_h_
#define _matrices_h_

typedef float matrice44[16];

// Generate a orthogonal projection matrix as defined at:
// http://www.opengl.org/sdk/docs/man/xhtml/glOrtho.xml
extern void ortho(matrice44 m, float left, float right, float bottom, float top, float near, float far);

#endif
