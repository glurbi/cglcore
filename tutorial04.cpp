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
 * In this tutorial, we render a rotating cube, with some diffuse lighting.
 * It uses a perspective projection for transforming the vertex positions.
 */

typedef float matrix44[16];

inline long currentTimeMillis() { return clock() / (CLOCKS_PER_SEC / 1000); }
const float pi = atan(1.0f) * 4.0f;
inline float toRadians(float degrees) { return degrees * pi / 180.0f; }

// Up to 16 attributes per vertex is allowed so any value between 0 and 15 will do.
const int POSITION_ATTRIBUTE_INDEX = 0;
const int NORMAL_ATTRIBUTE_INDEX = 1;

// defines the perspective projection volume
const float left = -1.5f;
const float right = 1.5f;
const float bottom = -1.5f;
const float top = 1.5f;
const float nearPlane = 1.0f;
const float farPlane = 10.0f;

int windowId; // the glut window id

bool initialized = false;
long startTimeMillis;
GLuint cubePositionsId;
GLuint cubeNormalsId;
GLuint programId;

float aspectRatio;
int frameCount;
int totalFrameCount;
int currentWidth;
int currentHeight;

void identity(matrix44 m) {
    for (int i = 0; i < 16; i++) {
        m[i] = 0.0f;
    }
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
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

void setSwapInterval(int interval) {
    if (glxewIsSupported("GLX_EXT_swap_control")) {
        Display *dpy = glXGetCurrentDisplay();
        GLXDrawable drawable = glXGetCurrentDrawable();
        glXSwapIntervalEXT(dpy, drawable, interval);
    }
}

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

void createProgram() {
    const GLchar* vertexShaderSource = readTextFile("tutorial04.vert");
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);
	checkShaderCompileStatus(vertexShaderId);

    const GLchar* fragmentShaderSource = readTextFile("tutorial04.frag");
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

void createCube() {
    float positions[] = {
        // back face
        1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        // front face
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        // bottom face
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        // top face
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, 
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        // left face
        -1.0f, -1.0f, -1.0f, 
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        // right face
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f
    };
    glGenBuffers(1, &cubePositionsId);
    glBindBuffer(GL_ARRAY_BUFFER, cubePositionsId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    float normals[] = {
        // back face
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        // front face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 
        // bottom face
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        // top face
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        // left face
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        // right face
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f
    };
    glGenBuffers(1, &cubeNormalsId);
    glBindBuffer(GL_ARRAY_BUFFER, cubeNormalsId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
}

void renderCube() {
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, cubePositionsId);
    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, cubeNormalsId);
    glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
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
    sprintf(title, "Tutorial04: %d FPS @ %d x %d", frameCount * 4, currentWidth, currentHeight);
    SDL_WM_SetCaption(title, title);
    frameCount = 0;
}

void render() {

    if (initialized == false) {
        createProgram();
        createCube();
        setSwapInterval(0);
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

    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glUseProgram(programId);

    //
    // calculate the ModelViewProjection and ModelViewProjection matrices
    //
    matrix44 tmp, mv, mvp, frustumMat, translateMat, rotateMat1, rotateMat2;
    frustum(frustumMat, left, right, bottom / aspectRatio, top / aspectRatio, nearPlane, farPlane);
    translate(translateMat, 0.0f, 0.0f, -3.0f);
    rotate(rotateMat1, 1.0f * elapsed / 100, 1.0f, 0.0f, 0.0f);
    rotate(rotateMat2, 1.0f * elapsed / 50, 0.0f, 1.0f, 0.0f);
    multm(tmp, rotateMat1, rotateMat2);
    multm(mv, translateMat, tmp);
    multm(mvp, frustumMat, mv);

    // set the uniforms before rendering
    GLuint mvpMatrixUniform = glGetUniformLocation(programId, "mvpMatrix");
    GLuint mvMatrixUniform = glGetUniformLocation(programId, "mvMatrix");
    GLuint colorUniform = glGetUniformLocation(programId, "color");
    GLuint lightDirUniform = glGetUniformLocation(programId, "lightDir");
    glUniformMatrix4fv(mvpMatrixUniform, 1, false, mvp);
    glUniformMatrix4fv(mvMatrixUniform, 1, false, mv);
    glUniform3f(colorUniform, 0.0f, 1.0f, 0.0f);
    glUniform3f(lightDirUniform, 0.0f, 0.0f, -1.0f);
    
	// render the cube
    renderCube();

    // display rendering buffer
    SDL_GL_SwapBuffers();
}

int main(int argc, char **argv) {

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL could not initialize.");
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);

    SDL_Surface* surfDisplay = SDL_SetVideoMode(800, 600, 32, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);
    if (surfDisplay == nullptr) {
        printf("Could not create SDL_Surface.");
        return 1;
    }

    glewInit(); // must be called AFTER the OpenGL context has been created
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
