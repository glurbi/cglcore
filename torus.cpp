#include <math.h>
#include <torus.h>
#include <stdlib.h>
#include <stdio.h>

static float pi = atan(1)*4;

static void torus(float u, float v, float r, float R, float **p) {
    // cf http://en.wikipedia.org/wiki/Torus
    **p = (R + r * cos(v)) * cos(u);
    (*p)++;
    **p = (R + r * cos(v)) * sin(u);
    (*p)++;
    **p = r * sin(v);
    (*p)++;
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
