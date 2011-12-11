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

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void render() {

    if (initialized == false) {
        createTriangles();
        initialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    renderTriangles();
}

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

// Helper to check for extension string presence.  Adapted from:
//   http://www.opengl.org/resources/features/OGLextensions/
bool isExtensionSupported(const char *extList, const char *extension) {

    const char *start;
    const char *where, *terminator;

    //Extension names should not have spaces.
    where = strchr(extension, ' ');
    if (where || *extension == '\0') {
        return false;
    }

    // It takes a bit of care to be fool-proof about parsing the OpenGL extensions string.
    // Don't be fooled by sub-strings, etc.
    for (start = extList; ;) {
        where = strstr(start, extension);

        if (!where) {
            break;
        }

        terminator = where + strlen(extension);

        if (where == start || *(where - 1) == ' ') {
            if (*terminator == ' ' || *terminator == '\0') {
                return true;
            }
        }

        start = terminator;
    }

    return false;
}

bool ctxErrorOccurred = false;
int ctxErrorHandler(Display* display, XErrorEvent* event) {
    ctxErrorOccurred = true;
    return 0;
}

// main method largely inspired from opengl wiki
// http://www.opengl.org/wiki/Tutorial%3a_OpenGL_3.0_Context_Creation_%28GLX%29
int main (int argc, char** argv) {

    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        printf("Failed to open X display\n");
        exit(1);
    }

    int visual_attribs[] = {
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_DOUBLEBUFFER    , True,
        None
    };

    int glx_major, glx_minor;
    if (!glXQueryVersion(display, &glx_major, &glx_minor) ||
       ((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1))
    {
        printf("Invalid GLX version");
        exit(1);
    }

    int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
    if (fbc == nullptr) {
        printf("Failed to retrieve a framebuffer config\n");
        exit(1);
    }

    // Pick the FB config/visual with the most samples per pixel
    int best_fbc = -1, best_num_samp = -1;
    for (int i = 0; i < fbcount; i++) {
        XVisualInfo *vi = glXGetVisualFromFBConfig(display, fbc[i]);
        if (vi != nullptr) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);
            if (best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
                best_fbc = i, best_num_samp = samples;
            }
        }
        XFree(vi);
    }
    GLXFBConfig bestFbc = fbc[best_fbc];
    XFree(fbc);

    XVisualInfo *vi = glXGetVisualFromFBConfig(display, bestFbc);
    XSetWindowAttributes swa;
    Colormap cmap;
    swa.colormap = cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
    swa.background_pixmap = None;
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask | KeyPressMask;
    Window win = XCreateWindow(display, RootWindow(display, vi->screen), 0, 0, 800, 600,
        0, vi->depth, InputOutput, vi->visual, CWBorderPixel|CWColormap|CWEventMask, &swa);
    if (!win) {
        printf("Failed to create window.\n");
        exit(1);
    }
    XFree(vi);

    XStoreName(display, win, "Tutorial 02");
    XMapWindow(display, win);

    // Get the default screen's GLX extension list
    const char *glxExts = glXQueryExtensionsString(display, DefaultScreen(display));
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");

    GLXContext ctx = nullptr;

    // Install an X error handler so the application won't exit if GL 3.0
    // context allocation fails.
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    // Check for the GLX_ARB_create_context extension string and the function.
    if (isExtensionSupported(glxExts, "GLX_ARB_create_context") && glXCreateContextAttribsARB) {
        int context_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 1,
            None
        };
        ctx = glXCreateContextAttribsARB(display, bestFbc, 0, True, context_attribs);

        // Sync to ensure any errors generated are processed.
        XSync(display, False);
        if (ctxErrorOccurred || ctx == nullptr) {
            printf("Could not create GL 3.0 context\n");
            exit(1);
        }
    }

    // Sync to ensure any errors generated are processed.
    XSync(display, False);
    // Restore the original error handler
    XSetErrorHandler(oldHandler);

    glXMakeCurrent(display, win, ctx);

    // must be called AFTER the OpenGL context has been created
    glewInit(); 

    reshape(800, 600);

    bool done = false;
    while (!done) {
        while (XPending(display) > 0) {
            XEvent event;
            XNextEvent(display, &event);
            switch (event.type) {
            case Expose:
                break;
            case ConfigureNotify:
                reshape(event.xconfigure.width, event.xconfigure.height);
                break;
            case KeyPress:
                done = true;
                break;
            }
        }
        render();
        glXSwapBuffers(display, win);
    }

    glXDestroyContext(display, ctx);
    XDestroyWindow(display, win);
    XFreeColormap(display, cmap);
    XCloseDisplay(display);
}

