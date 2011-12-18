#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <GL/glxew.h>

/*
 * In this tutorial, we render a rotating torus with ambient, diffuse and
 * specular light component, using Phong lighting (per vertex lighting).
 */

typedef float matrix44[16];

inline long currentTimeMillis() { return clock() / (CLOCKS_PER_SEC / 1000); }
const float pi = atan(1.0f) * 4.0f;
inline float toRadians(float degrees) { return degrees * pi / 180.0f; }

// determines the number of vertices in the torus
int n = 40;

inline int torusAttributeCount(int n) { return n * n * 6; }

// Up to 16 attributes per vertex is allowed so any value between 0 and 15 will do.
const int POSITION_ATTRIBUTE_INDEX = 0;
const int NORMAL_ATTRIBUTE_INDEX = 1;

// defines the perspective projection volume
const float left = -1.0f;
const float right = 1.0f;
const float bottom = -1.0f;
const float top = 1.0f;
const float nearPlane = 2.0f;
const float farPlane = 10.0f;

bool initialized = false;
long startTimeMillis;
GLuint programId;
GLuint torusPositionsId;
GLuint torusNormalsId;

float aspectRatio;
int frameCount;
int totalFrameCount;
int currentWidth;
int currentHeight;

char* readTextFile(const char* filename) {
    struct stat st;
    stat(filename, &st);
    int size = st.st_size;
    char* content = (char*) malloc((size+1)*sizeof(char));
    content[size] = 0;
    // we need to read as binary, not text, otherwise we are screwed on Windows
    FILE *file = fopen(filename, "rb");
    fread(content, 1, size, file);
    return content;
}

void checkShaderCompileStatus(GLuint shaderId) {
    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLint infoLogLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
        printf("Shader compilation failed...\n");
        char* log = (char*) malloc((1+infoLogLength)*sizeof(char));
        glGetShaderInfoLog(shaderId, infoLogLength, NULL, log);
        log[infoLogLength] = 0;
        printf("%s", log);
    }
}

void checkProgramLinkStatus(GLuint programId) {
    GLint linkStatus;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        GLint infoLogLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
        printf("Program link failed...\n");
        char* log = (char*) malloc((1+infoLogLength)*sizeof(char));
        glGetProgramInfoLog(programId, infoLogLength, NULL, log);
        log[infoLogLength] = 0;
        printf("%s", log);
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

void createProgram() {
    const GLchar* vertexShaderSource = readTextFile("tutorial07.vert");
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);
	checkShaderCompileStatus(vertexShaderId);

    const GLchar* fragmentShaderSource = readTextFile("tutorial07.frag");
    int fragmentShaderSourceLength = strlen(fragmentShaderSource);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, &fragmentShaderSourceLength);
    glCompileShader(fragmentShaderId);
	checkShaderCompileStatus(fragmentShaderId);

    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glBindAttribLocation(programId, POSITION_ATTRIBUTE_INDEX, "vPosition");
    glBindAttribLocation(programId, NORMAL_ATTRIBUTE_INDEX, "vNormal");
    glLinkProgram(programId);
    checkProgramLinkStatus(programId);
}

void torus(float u, float v, float r, float R, float **p) {
    // cf http://en.wikipedia.org/wiki/Torus
    (*p)[0] = (R + r * cos(v)) * cos(u);
    (*p)[1] = (R + r * cos(v)) * sin(u);
    (*p)[2] = r * sin(v);
    *p += 3;
}

void crossProduct(float* u, float* v, float* w) {
    w[0] = u[1]*v[2] - u[2]*v[1];
    w[1] = u[2]*v[0] - u[0]*v[2];
    w[2] = u[0]*v[1] - u[1]*v[0];
}

void normalize(float* v) {
    float norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] = v[0] / norm;
    v[1] = v[1] / norm;
    v[2] = v[2] / norm;
}

void torusNormal(int ui, int vi, int n, float r, float R, float **p) {
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

void renderTorus() {
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, torusPositionsId);
    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, torusNormalsId);
    glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, torusAttributeCount(n));
    glDisableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glDisableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    // we keep track of the aspect ratio to adjust the projection volume
    aspectRatio = 1.0f * width / height;
    currentWidth = width;
    currentHeight = height;
}

// thanks to http://openglbook.com/the-book/chapter-1-getting-started/#toc-measuring-performance
void timer(int value) {
    char title[512];
    sprintf(title, "Tutorial07: %d FPS @ %d x %d", frameCount * 4, currentWidth, currentHeight);
    SDL_WM_SetCaption(title, title);
    frameCount = 0;
}

void render() {

    if (initialized == false) {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        torusPositionsId = createTorusPositions(n, 0.3f, 1.0f);
        torusNormalsId = createTorusNormals(n, 0.3f, 1.0f);
        createProgram();
        startTimeMillis = currentTimeMillis();
        initialized = true;
    }

    frameCount++;
    totalFrameCount++;
    long now = currentTimeMillis();
    long elapsed = now - startTimeMillis;
    static long lastTimerCall = 0;
    if ((now - lastTimerCall) > 250) {
        timer(0);
        lastTimerCall = now;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(programId);

    //
    // calculate the ModelViewProjection and ModelViewProjection matrices
    //
    matrix44 tmp, mv, mvp, frustumMat, translateMat, rotateMat1, rotateMat2;
    frustum(frustumMat, left, right, bottom / aspectRatio, top / aspectRatio, nearPlane, farPlane);
    translate(translateMat, 0.0f, 0.0f, -5.0f);
    rotate(rotateMat1, 1.0f * elapsed / 50, 1.0f, 0.0f, 0.0f);
    rotate(rotateMat2, 1.0f * elapsed / 100, 0.0f, 1.0f, 0.0f);
    multm(tmp, rotateMat1, rotateMat2);
    multm(mv, translateMat, tmp);
    multm(mvp, frustumMat, mv);

    // set the uniforms before rendering
    GLuint mvpMatrixUniform = glGetUniformLocation(programId, "mvpMatrix");
    GLuint mvMatrixUniform = glGetUniformLocation(programId, "mvMatrix");
    GLuint colorUniform = glGetUniformLocation(programId, "color");
    GLuint ambientUniform = glGetUniformLocation(programId, "ambient");
    GLuint lightDirUniform = glGetUniformLocation(programId, "lightDir");
    glUniformMatrix4fv(mvpMatrixUniform, 1, false, mvp);
    glUniformMatrix4fv(mvMatrixUniform, 1, false, mv);
    glUniform3f(lightDirUniform, 1.0f, -1.0f, -1.0f);
    glUniform4f(colorUniform, 0.0f, 0.8f, 0.0f, 1.0f);
    glUniform4f(ambientUniform, 0.1f, 0.1f, 0.1f, 1.0f);

    // render!
    renderTorus();

    // display rendering buffer
    SDL_GL_SwapBuffers();
}

int main(int argc, char **argv) {

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);
    SDL_Surface* surfDisplay = SDL_SetVideoMode(800, 600, 32,
            SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);

    // must be called AFTER the OpenGL context has been created
    glewInit();
    reshape(800, 600);

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            }
        }
        render();
    }

    SDL_FreeSurface(surfDisplay);
    SDL_Quit();
    return 0;
}
