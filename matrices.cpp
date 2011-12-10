#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <matrices.hpp>

const float pi = atan(1.0f) * 4.0f;
inline float toRadians(float degrees) { return degrees * pi / 180.0f; }

float* Matrix44::raw() {
    return m;
}

MatrixStack44::MatrixStack44() {
    identity();
}

MatrixStack44& MatrixStack44::identity() {
    boost::shared_ptr<Matrix44> m(new Matrix44());
    ::identity(m->raw());
    mvp.push(m);
    mv.push(m);
    return *this;
}

MatrixStack44& MatrixStack44::ortho(float left, float right, float bottom, float top, float near, float far) {
    boost::shared_ptr<Matrix44> m(new Matrix44);
    ::ortho(m->raw(), left, right, bottom, top, near, far);
    mvp.push(m);
    return *this;
}

MatrixStack44& MatrixStack44::frustum(float left, float right, float bottom, float top, float near, float far) {
    boost::shared_ptr<Matrix44> m(new Matrix44());
    ::frustum(m->raw(), left, right, bottom, top, near, far);
    mvp.push(m);
    return *this;
}

MatrixStack44& MatrixStack44::translate(float x, float y, float z) {
    Matrix44 translation;
    ::translate(translation.raw(), x, y, z);
    boost::shared_ptr<Matrix44> m1(new Matrix44());
    multm(m1->raw(), mvp.top()->raw(), translation.raw());
    mvp.push(m1);
    boost::shared_ptr<Matrix44> m2(new Matrix44());
    multm(m2->raw(), mv.top()->raw(), translation.raw());
    mv.push(m2);
    return *this;
}

MatrixStack44& MatrixStack44::rotate(float a, float x, float y, float z) {
    Matrix44 rotation;
    ::rotate(rotation.raw(), a, x, y, z);
    boost::shared_ptr<Matrix44> m1(new Matrix44());
    multm(m1->raw(), mvp.top()->raw(), rotation.raw());
    mvp.push(m1);
    boost::shared_ptr<Matrix44> m2(new Matrix44());
    multm(m2->raw(), mv.top()->raw(), rotation.raw());
    mv.push(m2);
    return *this;
}

Matrix44& MatrixStack44::modelViewProjection() {
    return *mvp.top();
}

Matrix44& MatrixStack44::modelView() {
    return *mv.top();
}

void identity(matrix44 m) {
    for (int i = 0; i < 16; i++) {
        m[i] = 0.0f;
    }
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

void ortho(matrix44 m, float left, float right, float bottom, float top, float near, float far) {
    m[0] = 2 / (right - left);
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[5] = 2 / (top - bottom);
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 2 / (far - near);
    m[11] = 0.0f;
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
}

void multm(matrix44 m, matrix44 m1, matrix44 m2) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            m[i+j*4] =
                m1[i+0] * m2[j*4+0] +
                m1[i+4] * m2[j*4+1] +
                m1[i+8] * m2[j*4+2] +
                m1[i+12] * m2[j*4+3];
        }
    }
}

void multv(vector4 result, matrix44 m, vector4 v) {
    for (int i = 0; i < 4; i++) {
        result[i] =
            m[i+0] * v[0] +
            m[i+4] * v[1] +
            m[i+8] * v[2] +
            m[i+12] * v[3];
    }
}

void frustum(matrix44 m, float left, float right, float bottom, float top, float near, float far) {
    m[0] = 2 * near / (right - left);
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[5] = 2 * near / (top - bottom);
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = (right + left) / (right - left);
    m[9] = (top + bottom) / (top - bottom);
    m[10] = - (far + near) / (far - near);
    m[11] = -1.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = -2.0f * far * near / (far - near);
    m[15] = 0.0f;
}

void translate(matrix44 m, float x, float y, float z) {
    m[0] = 1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[5] = 1.0f;
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 1.0f;
    m[11] = 0.0f;
    m[12] = x;
    m[13] = y;
    m[14] = z;
    m[15] = 1.0f;
}

void rotate(matrix44 m, float a, float x, float y, float z) {
    float c = (float) cos(toRadians(a));
    float s = (float) sin(toRadians(a));
    m[0] = x * x * (1 - c) + c;
    m[1] = y * x * (1 - c) + z * s;
    m[2] = x * z * (1 - c) - y * s;
    m[3] = 0.0f;
    m[4] = y * x * (1 - c) - z * s;
    m[5] = y * y * (1 - c) + c;
    m[6] = y * z * (1 - c) + x * s;
    m[7] = 0.0f;
    m[8] = x * z * (1 - c) + y * s;
    m[9] = y * z * (1 - c) - x * s;
    m[10] = z * z * (1 - c) + c;
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void printm(matrix44 m) {
    printf("%f\t%f\t%f\t%f\t\n", m[0], m[4], m[8], m[12]);
    printf("%f\t%f\t%f\t%f\t\n", m[1], m[5], m[9], m[13]);
    printf("%f\t%f\t%f\t%f\t\n", m[2], m[6], m[10], m[14]);
    printf("%f\t%f\t%f\t%f\t\n", m[3], m[7], m[11], m[15]);
}

void printv(vector4 v) {
    printf("%f\t%f\t%f\t%f\t\n", v[0], v[1], v[2], v[3]);
}
