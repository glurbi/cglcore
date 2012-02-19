#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

//
// In this tutorial, we render a triangle and a quad using a shader program
// that forwards the vertex position without tranformation and invents some
// fragment color by using the vertex position.
//

// Up to 16 attributes per vertex is allowed so any value between 0 and 15 will do.
const int POSITION_ATTRIBUTE_INDEX = 0;

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

	//
	// compile the vertex shader
	//
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);
    checkShaderCompileStatus(vertexShaderId);

	//
	// compile the fragment shader
	//
    int fragmentShaderSourceLength = strlen(fragmentShaderSource);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, &fragmentShaderSourceLength);
    glCompileShader(fragmentShaderId);
    checkShaderCompileStatus(fragmentShaderId);

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
    checkProgramLinkStatus(programId);
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

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
}

void render() {

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

