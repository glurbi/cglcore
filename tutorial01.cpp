#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

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
GLuint trianglesId; // the triangles VBO id

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
    glGenBuffers(1, &trianglesId);
    glBindBuffer(GL_ARRAY_BUFFER, trianglesId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
}

// send the triangle vertices for drawing
void renderTriangles() {
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, trianglesId);
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
//    glutSwapBuffers();
}

// glut callback invoked each time a key has been pressed
void keyboardFunc(unsigned char key, int x, int y) {
//    glutDestroyWindow(windowId);
    exit(EXIT_SUCCESS);
}

static void make_window( Display *dpy, const char *name,
             int x, int y, int width, int height,
             Window *winRet, GLXContext *ctxRet)
{
   int attrib[] = { GLX_RGBA,
            GLX_RED_SIZE, 1,
            GLX_GREEN_SIZE, 1,
            GLX_BLUE_SIZE, 1,
            GLX_DOUBLEBUFFER,
            GLX_DEPTH_SIZE, 1,
            None };
   int scrnum;
   XSetWindowAttributes attr;
   unsigned long mask;
   Window root;
   Window win;
   GLXContext ctx;
   XVisualInfo *visinfo;

   scrnum = DefaultScreen( dpy );
   root = RootWindow( dpy, scrnum );

   visinfo = glXChooseVisual( dpy, scrnum, attrib );
   if (!visinfo) {
      printf("Error: couldn't get an RGB, Double-buffered visual\n");
      exit(1);
   }

   /* window attributes */
   attr.background_pixel = 0;
   attr.border_pixel = 0;
   attr.colormap = XCreateColormap( dpy, root, visinfo->visual, AllocNone);
   attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
   mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

   win = XCreateWindow( dpy, root, 0, 0, width, height,
                0, visinfo->depth, InputOutput,
                visinfo->visual, mask, &attr );

   /* set hints and properties */
   {
      XSizeHints sizehints;
      sizehints.x = x;
      sizehints.y = y;
      sizehints.width  = width;
      sizehints.height = height;
      sizehints.flags = USSize | USPosition;
      XSetNormalHints(dpy, win, &sizehints);
      XSetStandardProperties(dpy, win, name, name,
                              None, (char **)NULL, 0, &sizehints);
   }

   ctx = glXCreateContext( dpy, visinfo, NULL, True );
   if (!ctx) {
      printf("Error: glXCreateContext failed\n");
      exit(1);
   }

   XFree(visinfo);

   *winRet = win;
   *ctxRet = ctx;
}

static void
event_loop(Display *dpy, Window win)
{
   while (1) {
      while (XPending(dpy) > 0) {
         XEvent event;
         XNextEvent(dpy, &event);
         switch (event.type) {
     case Expose:
            /* we'll redraw below */
        break;
     case ConfigureNotify:
        reshapeFunc(event.xconfigure.width, event.xconfigure.height);
        break;
         case KeyPress:
                     return;
            }
         }
      }

      displayFunc();
      glXSwapBuffers(dpy, win);

}


int main(int argc, char **argv) {
	initialized = false;

	Display *dpy;
	Window win;
	GLXContext ctx;
	char *dpyName = NULL;
	dpy = XOpenDisplay(dpyName);
	if (!dpy) {
	    printf("Error: couldn't open display %s\n", dpyName);
	    return -1;
	}

	make_window(dpy, "glxgears", 0, 0, 800, 600, &win, &ctx);
	XMapWindow(dpy, win);
	glXMakeCurrent(dpy, win, ctx);
	reshapeFunc(800, 600);
	glewInit(); // must be called AFTER the OpenGL context has been created
	            // i.e. after glutCreateWindow has been called

//	init();
	event_loop(dpy, win);

	glXDestroyContext(dpy, ctx);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

//    glutInit(&argc, argv);
//    glutInitContextVersion(3, 3);
//    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
//    glutInitContextProfile(GLUT_CORE_PROFILE);
//    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
//    glutInitWindowSize(800, 600);
//    glutInitWindowPosition(0, 0);
//    windowId = glutCreateWindow("Tutorial 01");
//    glewInit(); // must be called AFTER the OpenGL context has been created
//				// i.e. after glutCreateWindow has been called
//    glutDisplayFunc(&displayFunc);
//    glutIdleFunc(&displayFunc);
//    glutReshapeFunc(&reshapeFunc);
//    glutKeyboardFunc(&keyboardFunc);
//    glutMainLoop();
    return EXIT_SUCCESS;
}
