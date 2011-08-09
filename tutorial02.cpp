#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

/*
 * In this tutorial, we render a triangle and a quad using a shader program
 * that forwards the vertex position without tranformation  and invents some
 * fragment color by using the vertex position.
 */

// Up to 16 attributes per vertex is allowed so any value between 0 and 15 will do.
const int POSITION_ATTRIBUTE_INDEX = 12;

int windowId; // the glut window id

bool initialized = false;
GLuint trianglesId;
GLuint quadId;
GLuint programId;

const GLchar* vertexShaderSource =
"#version 330\n"\

"in vec3 inPosition;\n"\
"out vec3 color;\n"\

"void main(void)\n"\
"{\n"\
"    gl_Position = vec4(inPosition, 1.0f);\n"\
"    color = inPosition;\n"\
"}\n";

const GLchar* fragmentShaderSource =
"#version 330\n"\

"in vec3 color;\n"\
"out vec4 outColor;\n"\

"void main(void)\n"\
"{\n"\
"    outColor = vec4(abs(color.xy), (1.0f-abs(color.x)), 1.0f);\n"\
"}\n";

void createProgram() {

	//
	// compile the vertex shader
	//
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);

	//
	// compile the fragment shader
	//
    int fragmentShaderSourceLength = strlen(fragmentShaderSource);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, &fragmentShaderSourceLength);
    glCompileShader(fragmentShaderId);

	//
	// link the shader program
	//
    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    // associates the "inPosition" variable from the vertex shader with the position attribute
    // the variable and the attribute must be bound before the program is linked
    glBindAttribLocation(programId, POSITION_ATTRIBUTE_INDEX, "inPosition");
    glLinkProgram(programId);
}

void createTriangle() {
    float positions[] = {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f
    };
    glGenBuffers(1, &trianglesId);
    glBindBuffer(GL_ARRAY_BUFFER, trianglesId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
}

void createQuad() {
    float positions[] = {
            0.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            -1.0f, 0.0f, 0.0f
    };
    glGenBuffers(1, &quadId);
    glBindBuffer(GL_ARRAY_BUFFER, quadId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
}

void renderTriangle() {
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, trianglesId);
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
}

void displayFunc() {

    if (initialized == false) {
        createProgram();
        createTriangle();
        createQuad();
        initialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(programId);
    renderTriangle();
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
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(0, 0);
    windowId = glutCreateWindow("Tutorial 02");
    glewInit(); // must be called AFTER the OpenGL context has been created
    glutDisplayFunc(&displayFunc);
    glutIdleFunc(&displayFunc);
    glutReshapeFunc(&reshapeFunc);
    glutKeyboardFunc(&keyboardFunc);
    glutMainLoop();
    return EXIT_SUCCESS;
}
