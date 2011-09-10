#include <math.h>
#include <torus.h>
#include <stdlib.h>
#include <stdio.h>

static float pi = atan(1)*4;

static void torus(float u, float v, float r, float R, float **p) {
    // cf http://en.wikipedia.org/wiki/Torus
    (*p)[0] = (R + r * cos(v)) * cos(u);
    (*p)[1] = (R + r * cos(v)) * sin(u);
    (*p)[2] = r * sin(v);
    *p += 3;
}

// TODO: move to utils
static void crossProduct(float* u, float* v, float* w) {
    w[0] = u[1]*v[2] - u[2]*v[1];
    w[1] = u[2]*v[0] - u[0]*v[2];
    w[2] = u[0]*v[1] - u[1]*v[0];
}

// TODO: move to utils
static void normalize(float* v) {
    float norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] = v[0] / norm;
    v[1] = v[1] / norm;
    v[2] = v[2] / norm;
}

static void torusNormal(int ui, int vi, int n, float r, float R, float **p) {
    float a = 2.0 * pi / n; // angle increment
    float f[12];
    float *q = f;
    torus(ui*a, (vi-1)*a, r, R, &q);
    torus(ui*a, (vi+1)*a, r, R, &q);
    torus((ui-1)*a, vi*a, r, R, &q);
    torus((ui+1)*a, vi*a, r, R, &q);
    float u[3] = { f[3]-f[0], f[4]-f[1], f[5]-f[2] };
    float v[3] = { f[6]-f[9], f[7]-f[10], f[8]-f[11] };
    float w[3];
    crossProduct(u, v, w);
    normalize(w);
    (*p)[0] = w[0];
    (*p)[1] = w[1];
    (*p)[2] = w[2];
    *p += 3;
}

GLuint createTorusPositions(int n, float r, float R) {
    int nfloats = torusAttributeCount(n)*3; // number of floats in the buffer
    int bufferSize = nfloats*sizeof(float);
    float *positions = (float*) malloc(bufferSize);
    float *p = positions;
    float a = 2.0 * pi / n; // angle increment
    for (int ui = 0; ui < n; ui++) {
        for (int vi = 0; vi < n; vi++) {
            torus(ui*a, vi*a, r, R, &p);
            torus((ui+1)*a, vi*a, r, R, &p);
            torus((ui+1)*a, (vi+1)*a, r, R, &p);

            torus(ui*a, vi*a, r, R, &p);
            torus((ui+1)*a, (vi+1)*a, r, R, &p);
            torus(ui*a, (vi+1)*a, r, R, &p);
        }
    }
    GLuint torusPositionsId;
    glGenBuffers(1, &torusPositionsId);
    glBindBuffer(GL_ARRAY_BUFFER, torusPositionsId);
    glBufferData(GL_ARRAY_BUFFER, nfloats*sizeof(float), positions, GL_STATIC_DRAW);
    free(positions);
    return torusPositionsId;
}

GLuint createTorusNormals(int n, float r, float R) {
    int nfloats = torusAttributeCount(n)*3; // number of floats in the buffer
    int bufferSize = nfloats*sizeof(float);
    float *normals = (float*) malloc(bufferSize);
    float *p = normals;
    for (int ui = 0; ui < n; ui++) {
        for (int vi = 0; vi < n; vi++) {
            torusNormal(ui, vi, n, r, R, &p);
            torusNormal(ui+1, vi, n, r, R, &p);
            torusNormal(ui+1, vi+1, n, r, R, &p);

            torusNormal(ui, vi, n, r, R, &p);
            torusNormal(ui+1, vi+1, n, r, R, &p);
            torusNormal(ui, vi+1, n, r, R, &p);
        }
    }
    GLuint torusNormalsId;
    glGenBuffers(1, &torusNormalsId);
    glBindBuffer(GL_ARRAY_BUFFER, torusNormalsId);
    glBufferData(GL_ARRAY_BUFFER, nfloats*sizeof(float), normals, GL_STATIC_DRAW);
    free(normals);
    return torusNormalsId;
}
