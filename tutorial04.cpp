#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <utils.h>
#include <matrices.h>
#include <files.h>
#include <glutils.h>

/*
 * In this tutorial, we render a rotating cube, with some diffuse lighting.
 * It uses a perspective projection for transforming the vextex positions.
 */

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

void reshapeFunc(int width, int height) {
    glViewport(0, 0, width, height);
    // we keep track of the aspect ratio to adjust the projection volume
    aspectRatio = 1.0f * width / height;
    currentWidth = width;
    currentHeight = height;
}

void displayFunc() {

    if (initialized == false) {
        createProgram();
        createCube();
        setSwapInterval(0);
        startTimeMillis = currentTimeMillis();
        initialized = true;
    }

    frameCount++;
    totalFrameCount++;
    long elapsed = currentTimeMillis() - startTimeMillis;


    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glUseProgram(programId);

    // calculate the ModelViewProjection and ModelViewProjection matrices
    matrix44 matrices[4];
    matrix44 *mvp;
    matrix44 *mv;
    frustum(&matrices[0], left, right, bottom / aspectRatio, top / aspectRatio, nearPlane, farPlane);
    translate(&matrices[1], 0.0f, 0.0f, -3.0f);
    rotate(&matrices[2], 1.0f * elapsed / 100, 1.0f, 0.0f, 0.0f);
    rotate(&matrices[3], 1.0f * elapsed / 50, 0.0f, 1.0f, 0.0f);
    mvp = multm(matrices[0], matrices[1]);
    mvp = multm(*mvp, matrices[2]);
    mvp = multm(*mvp, matrices[3]);
    mv = multm(matrices[1], matrices[2]);
    mv = multm(*mv, matrices[3]);

    // set the uniforms before rendering
    GLuint mvpMatrixUniform = glGetUniformLocation(programId, "mvpMatrix");
    GLuint mvMatrixUniform = glGetUniformLocation(programId, "mvMatrix");
    GLuint colorUniform = glGetUniformLocation(programId, "color");
    GLuint lightDirUniform = glGetUniformLocation(programId, "lightDir");
    glUniformMatrix4fv(mvpMatrixUniform, 1, false, *mvp);
    glUniformMatrix4fv(mvMatrixUniform, 1, false, *mv);
    glUniform3f(colorUniform, 0.0f, 1.0f, 0.0f);
    glUniform3f(lightDirUniform, 0.0f, 0.0f, -1.0f);
    
	// render the cube
    renderCube();

    // display rendering buffer
    glutSwapBuffers();

    // free resources
    free(mvp);
    free(mv);
}

void keyboardFunc(unsigned char key, int x, int y) {
    glutDestroyWindow(windowId);
    exit(EXIT_SUCCESS);
}

void idleFunc() {
    glutPostRedisplay();
}

// thanks to http://openglbook.com/the-book/chapter-1-getting-started/#toc-measuring-performance
void timerFunc(int value) {
    char title[512];
    sprintf(title, "Tutorial04: %d FPS @ %d x %d", frameCount * 4, currentWidth, currentHeight);
    glutSetWindowTitle(title);
    frameCount = 0;
    glutTimerFunc(250, timerFunc, 0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    // multisampling might not be supported by all hardware
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(0, 0);
    windowId = glutCreateWindow("Tutorial 04");
    // glewInit() must be called AFTER the OpenGL context has been created
    glewInit(); 
    glutDisplayFunc(&displayFunc);
    glutIdleFunc(&idleFunc);
    glutReshapeFunc(&reshapeFunc);
    glutKeyboardFunc(&keyboardFunc);
    glutTimerFunc(0, timerFunc, 0);
    totalFrameCount = 0;
    glutMainLoop();
    return EXIT_SUCCESS;
}
