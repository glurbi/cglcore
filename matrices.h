#ifndef _matrices_h_
#define _matrices_h_

#include <stack>

using namespace std;

typedef float matrix44[16];
typedef float vector4[4];

class Matrix44 {
public:
    Matrix44();
    float* raw();
private:
    float m[16];
};

class MatrixStack44 {
public:
    MatrixStack44();
    MatrixStack44& identity();
    MatrixStack44& ortho(float left, float right, float bottom, float top, float near, float far);
    MatrixStack44& frustum(float left, float right, float bottom, float top, float near, float far);
    MatrixStack44& translate(float x, float y, float z);
    MatrixStack44& rotate(float a, float x, float y, float z);
    Matrix44& modelViewProjection();
    Matrix44& modelView();
private:
    stack<Matrix44> mvp; // ModelViewProjection
    stack<Matrix44> mv; // ModelView
};

extern void identity(matrix44 m);

// Generate a orthogonal projection matrix as defined at:
// http://www.opengl.org/sdk/docs/man/xhtml/glOrtho.xml
extern void ortho(matrix44 m, float left, float right, float bottom, float top, float near, float far);

// m = m1 * m2
extern void multm(matrix44 m, matrix44 m1, matrix44 m2);

// result = m * v
extern void multv(vector4 result, matrix44 m, vector4 v);

// Generate a perspective projection matrix as defined at:
// http://www.opengl.org/sdk/docs/man/xhtml/glFrustum.xml
extern void frustum(matrix44 m, float left, float right, float bottom, float top, float near, float far);

// Generate a translation matrix as defined at:
// http://www.opengl.org/sdk/docs/man/xhtml/glTranslate.xml
extern void translate(matrix44 m, float x, float y, float z);

// Generate a rotation matrix as defined at:
// http://www.opengl.org/sdk/docs/man/xhtml/glRotate.xml
extern void rotate(matrix44 m, float a, float x, float y, float z);

extern void printm(matrix44 m);

extern void printv(vector4 v);

#endif
