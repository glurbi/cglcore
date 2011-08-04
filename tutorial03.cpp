#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <matrices.h>
#include <files.h>
#include <glutils.h>

// Up to 16 attributes per vertex is allowed so any value between 0 and 15 will do.
const int POSITION_ATTRIBUTE_INDEX = 0;

// defines the orthographic projection volume
const float left = -1.5f;
const float right = 1.5f;
const float bottom = -1.5f;
const float top = 1.5f;
const float nearPlane = 1.0f;
const float farPlane = -1.0f;

int windowId; // the glut window id

bool initialized = false;
GLuint triangleId;
GLuint quadId;
GLuint programId;
float aspectRatio;

void createProgram() {
    const GLchar* vertexShaderSource = readFile("tutorial03.vert");
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);
	checkShaderCompileStatus(vertexShaderId);

    const GLchar* fragmentShaderSource = readFile("tutorial03.frag");
    int fragmentShaderSourceLength = strlen(fragmentShaderSource);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, &fragmentShaderSourceLength);
    glCompileShader(fragmentShaderId);
	checkShaderCompileStatus(fragmentShaderId);

    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    // associates the "inPosition" variable from the vertex shader with the position attribute
    // the variable and the attribute must be bound before the program is linked
    glBindAttribLocation(programId, POSITION_ATTRIBUTE_INDEX, "position");
    glLinkProgram(programId);
    checkProgramLinkStatus(programId);
}

void createTriangle() {
    float positions[] = {
            -0.5f, -0.5f, 0.0f,
            1.0f, -0.5f, 0.0f,
            -0.5f, 1.0f, 0.0f
    };
    glGenBuffers(1, &triangleId);
    glBindBuffer(GL_ARRAY_BUFFER, triangleId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
}

void createQuad() {
    float positions[] = {
            0.5f, 0.5f, 0.0f,
            -1.0f, 0.5f, 0.0f,
            0.5f, -1.0f, 0.0f,
            0.5f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            -1.0f, 0.5f, 0.0f
    };
    glGenBuffers(1, &quadId);
    glBindBuffer(GL_ARRAY_BUFFER, quadId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
}

void renderTriangle() {
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, triangleId);
    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
}

void renderQuad() {
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, quadId);
    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
}

void reshapeFunc(int width, int height) {
    glViewport(0, 0, width, height);
    // we keep track of the aspect ratio to adjust the projection volume
    aspectRatio = 1.0f * width / height;
}

void displayFunc() {

    if (initialized == false) {
        createProgram();
        createTriangle();
        createQuad();
        initialized = true;
    }

	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programId);

    // defines the model view projection matrix and set the corresponding uniform
    // NB: bottom and top are adjusted with the aspect ratio
    matrix44 mvp;
    ortho(mvp, left, right, bottom / aspectRatio, top / aspectRatio, nearPlane, farPlane);
    GLuint matrixUniform = glGetUniformLocation(programId, "mvpMatrix");
    glUniformMatrix4fv(matrixUniform, 1, false, mvp);

	// we need the location of the uniform in order to set its value
    GLuint color = glGetUniformLocation(programId, "color");

	// render the triangle in yellow
    glUniform4f(color, 1.0f, 1.0f, 0.0f, 0.7f);
    renderTriangle();

	// render the quad in blue
    glUniform4f(color, 0.2f, 0.2f, 1.0f, 0.7f);
    renderQuad();

    glutSwapBuffers();
}

void keyboardFunc(unsigned char key, int x, int y) {
    glutDestroyWindow(windowId);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(0, 0);
    windowId = glutCreateWindow("Tutorial 03");
    glewInit(); // must be called AFTER the OpenGL context has been created
    glutDisplayFunc(&displayFunc);
    glutIdleFunc(&displayFunc);
    glutReshapeFunc(&reshapeFunc);
    glutKeyboardFunc(&keyboardFunc);
    glutMainLoop();
    return EXIT_SUCCESS;
}
