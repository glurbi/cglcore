#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

/*
 * In this tutorial, we render triangles without specifying a shader program.
 * Even if we are using the core profile (i.e. no more fixed pipeline), the
 * triangles are rendered in white on my nvidia card.
 * The triangles are truncated. That is the effect of the clipping volume:
 * xmin=-1.0f xmax=1.0f ymin=-1.0f ymax=1.0f zmin=-1.0f zmax=1.0f
 */

// the core "fixed" pipeline (or nvidia?) seems to expect the position at the
// following index...
const int POSITION_ATTRIBUTE_INDEX = 0;

int windowId; // the glut window id
bool initialized; // have we initialized the buffer objects?
GLuint triangleId; // the triangles VBO id TODO: rename to trianglesId

// create the triangle vertex buffer
void createTriangles() {
    float positions[3*3*9];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            positions[i*27+j*9+0] = i*1.0f + 0.0f - 1.25f; // x1
            positions[i*27+j*9+1] = j*1.0f + 0.0f - 1.25f; // y1
            positions[i*27+j*9+2] = 0.0f; // z1
            positions[i*27+j*9+3] = i*1.0f + 1.0f - 1.25f; // x2
            positions[i*27+j*9+4] = j*1.0f + 0.0f - 1.25f; // y2
            positions[i*27+j*9+5] = 0.0f; // z2
            positions[i*27+j*9+6] = i*1.0f + 0.0f - 1.25f; // x3
            positions[i*27+j*9+7] = j*1.0f + 1.0f - 1.25f; // y3
            positions[i*27+j*9+8] = 0.0f; // z3
        }
    }
    glGenBuffers(1, &triangleId);
    glBindBuffer(GL_ARRAY_BUFFER, triangleId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
}

// send the triangle vertices for drawing
void renderTriangles() {
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, triangleId);
    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 27);
    glDisableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
}

// glut callback invoked when the window is resized
void reshapeFunc(int width, int height) {
    glViewport(0, 0, width, height);
}

// glut callback invoked each time the opengl buffer must be painted
void displayFunc() {

    if (initialized == false) {
        createTriangles();
        initialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    renderTriangles();
    glutSwapBuffers();
}

// glut callback invoked each time a key has been pressed
void keyboardFunc(unsigned char key, int x, int y) {
    glutDestroyWindow(windowId);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
	initialized = false;
    glutInit(&argc, argv);
    glutInitContextVersion(3, 3);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(0, 0);
    windowId = glutCreateWindow("Tutorial 01");
    glewInit(); // must be called AFTER the OpenGL context has been created
				// i.e. after glutCreateWindow has been called
    glutDisplayFunc(&displayFunc);
    glutIdleFunc(&displayFunc);
    glutReshapeFunc(&reshapeFunc);
    glutKeyboardFunc(&keyboardFunc);
    glutMainLoop();
    return EXIT_SUCCESS;
}
