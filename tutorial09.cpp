#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <GL/glxew.h>
#include <vector>

/*
 * In this tutorial, we render a rotating sphere lighted with ambient
 * and diffuse light component, using gouraud lighting and flat shading.
 */

// C/C++ does not have a default definition for pi!
const float pi = atan(1.0f) * 4.0f;

// defines the perspective projection volume
const float left = -1.0f;
const float right = 1.0f;
const float bottom = -1.0f;
const float top = 1.0f;
const float nearPlane = 2.0f;
const float farPlane = 10.0f;

class matrix44 {
public:
	matrix44 multm(const matrix44& m2) {
		matrix44 m;
		for (int i = 0; i < 4; i++) {
		    for (int j = 0; j < 4; j++) {
		        m.f[i+j*4] =
		            f[i+0] * m2.f[j*4+0] +
		            f[i+4] * m2.f[j*4+1] +
		            f[i+8] * m2.f[j*4+2] +
		            f[i+12] * m2.f[j*4+3];
		    }
		}
		return m;
	}
	float f[16];
};

class vector3 {
public:
    vector3(float x, float y, float z): x(x), y(y), z(z) {}
    void dump(float** p) { (*p)[0] = x; (*p)[1] = y; (*p)[2] = z; *p += 3; }
    vector3 normalize() {
        float norm = sqrt(x*x + y*y + z*z);
        return vector3(x / norm, y / norm, z / norm);
    }
    const float x, y, z;
};

class triangle {
public:
    triangle(vector3 p1, vector3 p2, vector3 p3): p1(p1), p2(p2), p3(p3) {}
    void dump(float** p) { p1.dump(p); p2.dump(p); p3.dump(p); }
    vector3 p1, p2, p3;
};

vector3 midPoint(vector3 p1, vector3 p2) {
    return vector3((p1.x + p2.x) / 2, (p1.y + p2.y) / 2, (p1.z + p2.z) / 2);
}

inline long currentTimeMillis() {
	return clock() / (CLOCKS_PER_SEC / 1000);
}
inline float toRadians(float degrees) { return degrees * pi / 180.0f; }

// determines the number iterations for
int n = 4;

inline int sphereAttributeCount(int n) { return 8 * pow(4, n) * 3; }

// Up to 16 attributes per vertex is allowed so any value between 0 and 15 will do.
const int POSITION_ATTRIBUTE_INDEX = 0;
const int NORMAL_ATTRIBUTE_INDEX = 1;


bool initialized = false;
long startTimeMillis;
GLuint programId;
GLuint spherePositionsId;
GLuint sphereNormalsId;

float aspectRatio;
int frameCount;
int totalFrameCount;
int currentWidth;
int currentHeight;

float* positions;

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

matrix44 frustum(float left, float right, float bottom, float top, float near, float far) {
	matrix44 frustumMatrix;
	float* m = frustumMatrix.f;
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
	return frustumMatrix;
}

matrix44 translate(float x, float y, float z) {
	matrix44 translateMatrix;
	float* m = translateMatrix.f;
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
	return translateMatrix;
}

matrix44 rotate(float a, float x, float y, float z) {
	matrix44 rotateMatrix;
	float* m = rotateMatrix.f;
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
	return rotateMatrix;
}

void refine(int depth, triangle t, float** p) {
    if (depth == n) {
        t.dump(p);
    } else {
        vector3 m1 = midPoint(t.p2, t.p3).normalize();
        vector3 m2 = midPoint(t.p3, t.p1).normalize();
        vector3 m3 = midPoint(t.p1, t.p2).normalize();
        refine(depth + 1, triangle(t.p1, m3, m2), p);
        refine(depth + 1, triangle(m3, t.p2, m1), p);
        refine(depth + 1, triangle(m1, m2, m3), p);
        refine(depth + 1, triangle(m2, m1, t.p3), p);
    }
}

GLuint createSpherePositions() {
    int nfloats = sphereAttributeCount(n)*3; // number of floats in the buffer
    int bufferSize = nfloats*sizeof(float);
    positions = (float*) malloc(bufferSize);
    float* p = positions;
    GLuint spherePositionsId;

    //
    // we refine each side of an octahedron
    // cf http://paulbourke.net/miscellaneous/sphere_cylinder/
    //
    refine(0, triangle(vector3(0.0f, 1.0f, 0.0f), vector3(0.0f, 0.0f, 1.0f), vector3(1.0f, 0.0f, 0.0f)), &p);
    refine(0, triangle(vector3(0.0f, 1.0f, 0.0f), vector3(1.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, -1.0f)), &p);
    refine(0, triangle(vector3(0.0f, 1.0f, 0.0f), vector3(0.0f, 0.0f, -1.0f), vector3(-1.0f, 0.0f, 0.0f)), &p);
    refine(0, triangle(vector3(0.0f, 1.0f, 0.0f), vector3(-1.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, 1.0f)), &p);
    refine(0, triangle(vector3(0.0f, -1.0f, 0.0f), vector3(1.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, 1.0f)), &p);
    refine(0, triangle(vector3(0.0f, -1.0f, 0.0f), vector3(0.0f, 0.0f, 1.0f), vector3(-1.0f, 0.0f, 0.0f)), &p);
    refine(0, triangle(vector3(0.0f, -1.0f, 0.0f), vector3(-1.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, -1.0f)), &p);
    refine(0, triangle(vector3(0.0f, -1.0f, 0.0f), vector3(0.0f, 0.0f, -1.0f), vector3(1.0f, 0.0f, 0.0f)), &p);

    glGenBuffers(1, &spherePositionsId);
    glBindBuffer(GL_ARRAY_BUFFER, spherePositionsId);
    glBufferData(GL_ARRAY_BUFFER, nfloats*sizeof(float), positions, GL_STATIC_DRAW);
    //free(positions);
    return spherePositionsId;
}

GLuint createSphereNormals(float* positions) {
    int nfloats = sphereAttributeCount(n)*3; // number of floats in the buffer
    int bufferSize = nfloats*sizeof(float);
    float* normals = (float*) malloc(bufferSize);
    float* n = normals;
    float* p = positions;
    GLuint sphereNormalsId;

    for (int i = 0; i < nfloats; i+=9) {
        float nx = (p[i+0] + p[i+3] + p[i+6]) / 3;
        float ny = (p[i+1] + p[i+4] + p[i+7]) / 3;
        float nz = (p[i+2] + p[i+5] + p[i+8]) / 3;
        n[i+0] = n[i+3] = n[i+6] = nx;
        n[i+1] = n[i+4] = n[i+7] = ny;
        n[i+2] = n[i+5] = n[i+8] = nz;
    }

    glGenBuffers(1, &sphereNormalsId);
    glBindBuffer(GL_ARRAY_BUFFER, sphereNormalsId);
    glBufferData(GL_ARRAY_BUFFER, nfloats*sizeof(float), normals, GL_STATIC_DRAW);
    free(normals);
    return sphereNormalsId;
}

void createProgram() {
    const GLchar* vertexShaderSource = readTextFile("tutorial08.vert");
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);
	checkShaderCompileStatus(vertexShaderId);

    const GLchar* fragmentShaderSource = readTextFile("tutorial08.frag");
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
    sprintf(title, "Tutorial08: %d FPS @ %d x %d", frameCount * 4, currentWidth, currentHeight);
    SDL_WM_SetCaption(title, title);
    frameCount = 0;
}

void renderSphere() {
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, spherePositionsId);
    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, sphereNormalsId);
    glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, sphereAttributeCount(n));
    glDisableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glDisableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render() {

    if (initialized == false) {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        spherePositionsId = createSpherePositions();
        sphereNormalsId = createSphereNormals(positions);
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
    frustumMat = frustum(left, right, bottom / aspectRatio, top / aspectRatio, nearPlane, farPlane);
    translateMat = translate(0.0f, 0.0f, -3.0f);
    rotateMat1 = rotate(1.0f * elapsed / 50, 1.0f, 0.0f, 0.0f);
    rotateMat2 = rotate(1.0f * elapsed / 100, 0.0f, 1.0f, 0.0f);
    tmp = rotateMat1.multm(rotateMat2);
    mv = translateMat.multm(tmp);
    mvp = frustumMat.multm(mv);

    // set the uniforms before rendering
    GLuint mvpMatrixUniform = glGetUniformLocation(programId, "mvpMatrix");
    GLuint mvMatrixUniform = glGetUniformLocation(programId, "mvMatrix");
    GLuint colorUniform = glGetUniformLocation(programId, "color");
    GLuint ambientUniform = glGetUniformLocation(programId, "ambient");
    GLuint lightDirUniform = glGetUniformLocation(programId, "lightDir");
    glUniformMatrix4fv(mvpMatrixUniform, 1, false, mvp.f);
    glUniformMatrix4fv(mvMatrixUniform, 1, false, mv.f);
    glUniform3f(lightDirUniform, 1.0f, -1.0f, -1.0f);
    glUniform4f(colorUniform, 0.5f, 0.5f, 0.5f, 1.0f);
    glUniform4f(ambientUniform, 0.1f, 0.1f, 0.1f, 1.0f);

    // render!
    renderSphere();

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
    SDL_Surface* surfDisplay = SDL_SetVideoMode(900, 900, 32,
            SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);

    // must be called AFTER the OpenGL context has been created
    glewInit();
    reshape(900, 900);

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                event.type == SDL_KEYDOWN ||
                event.type == SDL_KEYUP)
            {
                done = true;
            }
        }
        render();
    }

    SDL_FreeSurface(surfDisplay);
    SDL_Quit();
    return 0;
}
