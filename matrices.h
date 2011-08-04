#ifndef _matrices_h_
#define _matrices_h_

typedef float matrix44[16];
typedef float vector4[4];

// Generate a orthogonal projection matrix as defined at:
// http://www.opengl.org/sdk/docs/man/xhtml/glOrtho.xml
extern void ortho(matrix44 m, float left, float right, float bottom, float top, float near, float far);

extern void identity(matrix44 m);

extern void multm(matrix44 m1, matrix44 m2, matrix44 result);

extern void multv(matrix44 m, vector4 v, vector4 result);

// Generate a perspective projection matrix as defined at:
// http://www.opengl.org/sdk/docs/man/xhtml/glFrustum.xml
extern void frustum(matrix44 m, float left, float right, float bottom, float top, float near, float far);

// Generate a translation matrix as defined at:
// http://www.opengl.org/sdk/docs/man/xhtml/glTranslate.xml
extern void translate(matrix44 m, float x, float y, float z);

// Generate a rotation matrix as defined at:
// http://www.opengl.org/sdk/docs/man/xhtml/glRotate.xml
extern void rotate(matrix44 m, float a, float x, float y, float z);

extern void print(matrix44 m);

extern void print(vector4 v);

#endif
