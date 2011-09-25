#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <utils.h>
#include <matrices.h>
#include <files.h>
#include <glutils.h>
#include <torus.h>

/*
 * In this tutorial, we render a rotating torus with ambient, diffuse and
 * specular light component, using Gouraud lighting (per vertex lighting).
 */

// determines the number of vertices in the torus
int n = 30;

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

int windowId; // the glut window id

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

void createProgram() {
    const GLchar* vertexShaderSource = readTextFile("tutorial06.vert");
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);
	checkShaderCompileStatus(vertexShaderId);

    const GLchar* fragmentShaderSource = readTextFile("tutorial06.frag");
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

void reshapeFunc(int width, int height) {
    glViewport(0, 0, width, height);
    // we keep track of the aspect ratio to adjust the projection volume
    aspectRatio = 1.0f * width / height;
    currentWidth = width;
    currentHeight = height;
}

void displayFunc() {

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
    long elapsed = currentTimeMillis() - startTimeMillis;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(programId);

    // calculate the ModelViewProjection and ModelViewProjection matrices
    MatrixStack44 mstack;
    mstack.frustum(left, right, bottom / aspectRatio, top / aspectRatio, nearPlane, farPlane)
          .translate(0.0f, 0.0f, -5.0f)
          .rotate(1.0f * elapsed / 30, 1.0f, 0.0f, 0.0f)
          .rotate(1.0f * elapsed / 10, 0.0f, 1.0f, 0.0f);

    // set the uniforms before rendering
    GLuint mvpMatrixUniform = glGetUniformLocation(programId, "mvpMatrix");
    GLuint mvMatrixUniform = glGetUniformLocation(programId, "mvMatrix");
    GLuint colorUniform = glGetUniformLocation(programId, "color");
    GLuint ambientUniform = glGetUniformLocation(programId, "ambient");
    GLuint lightDirUniform = glGetUniformLocation(programId, "lightDir");
    glUniformMatrix4fv(mvpMatrixUniform, 1, false, mstack.modelViewProjection().raw());
    glUniformMatrix4fv(mvMatrixUniform, 1, false, mstack.modelView().raw());
    glUniform3f(lightDirUniform, 1.0f, -1.0f, -1.0f);
    glUniform4f(colorUniform, 0.8f, 0.0f, 0.0f, 1.0f);
    glUniform4f(ambientUniform, 0.1f, 0.1f, 0.1f, 1.0f);

    // render!
    renderTorus();

    // display rendering buffer
    glutSwapBuffers();
}

void keyboardFunc(unsigned char key, int x, int y) {
    glutDestroyWindow(windowId);
    exit(EXIT_SUCCESS);
}

void idleFunc() {
    glutPostRedisplay();
}

// TODO: move to glutils.cpp.h
// thanks to http://openglbook.com/the-book/chapter-1-getting-started/#toc-measuring-performance
void timerFunc(int value) {
    char title[512];
    sprintf(title, "Tutorial06: %d FPS @ %d x %d", frameCount * 4, currentWidth, currentHeight);
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
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(0, 0);
    windowId = glutCreateWindow("Tutorial 06");
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
