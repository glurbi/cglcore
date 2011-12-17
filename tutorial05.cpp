#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <png.h>
#include <GL/glew.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkgl.h>

/*
 * In this tutorial, we render a rotating cube with a transparent texture.
 * It demonstrates how to activate and use a texture unit in a shader program.
 */

typedef float matrix44[16];

inline long currentTimeMillis() { return clock() / (CLOCKS_PER_SEC / 1000); }
const float pi = atan(1.0f) * 4.0f;
inline float toRadians(float degrees) { return degrees * pi / 180.0f; }

// Up to 16 attributes per vertex is allowed so any value between 0 and 15 will do.
const int POSITION_ATTRIBUTE_INDEX = 0;
const int NORMAL_ATTRIBUTE_INDEX = 1;
const int TEXCOORD_ATTRIBUTE_INDEX = 2;

// defines the perspective projection volume
const float left = -1.0f;
const float right = 1.0f;
const float bottom = -1.0f;
const float top = 1.0f;
const float nearPlane = 2.0f;
const float farPlane = 10.0f;

GtkWidget *window;
guint idle_id = 0;

bool initialized = false;
long startTimeMillis;
GLuint programId;
GLuint textureId;
GLuint cubePositionsId;
GLuint cubeNormalsId;
GLuint cubeTexCoordsId;

float aspectRatio;
int frameCount;
int totalFrameCount;
int currentWidth;
int currentHeight;

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

char* readPngFile(const char* filename, int *width, int *height, GLenum *format) {
    FILE *fp;
    png_structp png_ptr;
    png_infop info_ptr;
    fp = fopen(filename, "rb");

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    if (info_ptr->color_type == PNG_COLOR_TYPE_RGB) {
        *format = GL_RGB;
    } else if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
        *format = GL_RGBA;
    } else {
        printf("Unsupported color type in readPngFile(%s)", filename);
        return NULL;
    }

    *width = info_ptr->width;
    *height = info_ptr->height;

    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    png_bytep image_data = (png_bytep) malloc(rowbytes * info_ptr->height);
    png_bytepp row_pointers = (png_bytepp) malloc(info_ptr->height * sizeof(png_bytep));

    for (unsigned int i = 0; i < info_ptr->height; ++i) {
      row_pointers[info_ptr->height - 1 - i] = image_data + i * rowbytes;
    }

    png_read_image(png_ptr, row_pointers);
    //png_read_destroy(png_ptr, info_ptr, (png_infop)0);
    free(png_ptr);
    free(info_ptr);
    free(row_pointers);
    fclose(fp);
    return (char*) image_data;
}

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
    float texcoords[] = {
        // back face
        1.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        // front face
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        // bottom face
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        // top face
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f,  0.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        // left face
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        // right face
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };
    glGenBuffers(1, &cubeTexCoordsId);
    glBindBuffer(GL_ARRAY_BUFFER, cubeTexCoordsId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
}

void createProgram() {
    const GLchar* vertexShaderSource = readTextFile("tutorial05.vert");
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);
	checkShaderCompileStatus(vertexShaderId);

    const GLchar* fragmentShaderSource = readTextFile("tutorial05.frag");
    int fragmentShaderSourceLength = strlen(fragmentShaderSource);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, &fragmentShaderSourceLength);
    glCompileShader(fragmentShaderId);
	checkShaderCompileStatus(fragmentShaderId);

    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glBindAttribLocation(programId, POSITION_ATTRIBUTE_INDEX, "pos");
    glBindAttribLocation(programId, NORMAL_ATTRIBUTE_INDEX, "normal");
    glBindAttribLocation(programId, TEXCOORD_ATTRIBUTE_INDEX, "texcoord");
    glLinkProgram(programId);
    checkProgramLinkStatus(programId);
}

void createTexture() {
    int width, height;
    GLenum format;
    char* data = readPngFile("tux.png", &width, &height, &format);
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
}

void renderCube() {
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, cubePositionsId);
    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, cubeNormalsId);
    glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(TEXCOORD_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, cubeTexCoordsId);
    glVertexAttribPointer(TEXCOORD_ATTRIBUTE_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDisableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
    glDisableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
    glDisableVertexAttribArray(TEXCOORD_ATTRIBUTE_INDEX);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

gboolean reshape(GtkWidget* widget, GdkEventConfigure* event, gpointer data) {
    int width = widget->allocation.width;
    int height = widget->allocation.height;
    glViewport(0, 0, width, height);
    // we keep track of the aspect ratio to adjust the projection volume
    aspectRatio = 1.0f * width / height;
    currentWidth = width;
    currentHeight = height;
    return TRUE;
}

// thanks to http://openglbook.com/the-book/chapter-1-getting-started/#toc-measuring-performance
void timer(int value) {
    char title[512];
    sprintf(title, "Tutorial05: %d FPS @ %d x %d", frameCount * 4, currentWidth, currentHeight);
    gtk_window_set_title(GTK_WINDOW(window), title);
    frameCount = 0;
}

gboolean draw(GtkWidget* widget, GdkEventExpose* event, gpointer data) {

    static GdkGLContext* glcontext;
    static GdkGLDrawable* gldrawable;

    if (initialized == false) {
        glcontext = gtk_widget_get_gl_context(widget);
        gldrawable = gtk_widget_get_gl_drawable (widget);
        gdk_gl_drawable_gl_begin(gldrawable, glcontext);
        glewInit(); // must be called AFTER the OpenGL context has been created

        glEnable(GL_TEXTURE_2D);
        glEnable (GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glCullFace(GL_CULL_FACE);
        createProgram();
        createTexture();
        createCube();
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
    frustum(frustumMat, left, right, bottom / aspectRatio, top / aspectRatio, nearPlane, farPlane);
    translate(translateMat, 0.0f, 0.0f, -5.0f);
    rotate(rotateMat1, 1.0f * elapsed / 100, 1.0f, 0.0f, 0.0f);
    rotate(rotateMat2, 1.0f * elapsed / 50, 0.0f, 1.0f, 0.0f);
    multm(tmp, rotateMat1, rotateMat2);
    multm(mv, translateMat, tmp);
    multm(mvp, frustumMat, mv);

    // activate the texture
    glBindTexture(GL_TEXTURE_2D, textureId);
    glActiveTexture(GL_TEXTURE0);

    // set the uniforms before rendering
    GLuint mvpMatrixUniform = glGetUniformLocation(programId, "mvpMatrix");
    GLuint mvMatrixUniform = glGetUniformLocation(programId, "mvMatrix");
    GLuint colorUniform = glGetUniformLocation(programId, "color");
    GLuint textureUniform = glGetUniformLocation(programId, "texture");
    GLuint lightDirUniform = glGetUniformLocation(programId, "lightDir");
    glUniformMatrix4fv(mvpMatrixUniform, 1, false, mvp);
    glUniformMatrix4fv(mvMatrixUniform, 1, false, mv);
    glUniform3f(lightDirUniform, 0.0f, 0.0f, -1.0f);
    glUniform3f(colorUniform, 0.0f, 1.0f, 1.0f);
    glUniform1i(textureUniform, 0);

    // render!
    glDisable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);
    renderCube();
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    renderCube();

    // display rendering buffer
    gdk_gl_drawable_swap_buffers(gldrawable);
    return TRUE;
}

gboolean idle(GtkWidget* widget) {
    gdk_window_invalidate_rect(widget->window, &widget->allocation, FALSE);
    gdk_window_process_updates(widget->window, FALSE);
    return TRUE;
}


void idle_add(GtkWidget* widget) {
    if (idle_id == 0) {
        idle_id = g_idle_add_full(GDK_PRIORITY_REDRAW, (GSourceFunc) idle, widget, NULL);
    }
}

void idle_remove(GtkWidget* widget) {
    if (idle_id != 0) {
        g_source_remove(idle_id);
        idle_id = 0;
    }
}

gboolean map(GtkWidget* widget, GdkEventAny* event, gpointer data) {
    idle_add(widget);
    return TRUE;
}

gboolean unmap(GtkWidget* widget, GdkEventAny* event, gpointer data) {
    idle_remove(widget);
    return TRUE;
}

static gboolean key(GtkWidget* widget, GdkEventKey* event, gpointer data) {
    GdkGLDrawable* gldrawable = gtk_widget_get_gl_drawable(widget);
    gdk_gl_drawable_gl_end(gldrawable);
    exit(0);
}

int main(int argc, char **argv) {

    GdkGLConfig* glconfig;
    GtkWidget* drawing_area;

    gtk_init(&argc, &argv);
    gtk_gl_init(&argc, &argv);

    glconfig = gdk_gl_config_new_by_mode((GdkGLConfigMode)
            (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE | GDK_GL_MODE_MULTISAMPLE));

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_reallocate_redraws(GTK_CONTAINER(window), TRUE);
    drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    gtk_widget_set_size_request(drawing_area, 800, 600);
    gtk_widget_set_gl_capability(drawing_area, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
    gtk_widget_add_events(drawing_area, GDK_VISIBILITY_NOTIFY_MASK);

    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "configure_event", G_CALLBACK(reshape), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "expose_event", G_CALLBACK(draw), NULL);
    g_signal_connect_swapped(G_OBJECT(window), "key_press_event", G_CALLBACK(key), drawing_area);
    g_signal_connect(G_OBJECT(drawing_area), "map_event", G_CALLBACK(map), NULL);
    g_signal_connect(G_OBJECT(drawing_area), "unmap_event", G_CALLBACK(unmap), NULL);

    gtk_widget_show(drawing_area);
    gtk_widget_show(window);
    gtk_main();
    return 0;
}
