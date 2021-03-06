#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include <GL/glew.h>

/*
 * In this tutorial, we render a triangle and a quad that overlap. It uses some
 * transparency effect and demonstrates the use of a uniform for specifying the
 * color of the geometries. In addition, the vertex positions are transformed
 * using an orthographic projection.
 */

typedef float matrix44[16];

// Up to 16 attributes per vertex is allowed so any value between 0 and 15 will do.
const int POSITION_ATTRIBUTE_INDEX = 0;

// defines the orthographic projection volume
const float left = -1.5f;
const float right = 1.5f;
const float bottom = -1.5f;
const float top = 1.5f;
const float nearPlane = 1.0f;
const float farPlane = -1.0f;

bool initialized = false;
GLuint trianglesId;
GLuint quadId;
GLuint programId;
float aspectRatio;

void ortho(matrix44 m, float left, float right, float bottom, float top, float near, float far) {
    m[0] = 2 / (right - left);
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;
    m[4] = 0.0f;
    m[5] = 2 / (top - bottom);
    m[6] = 0.0f;
    m[7] = 0.0f;
    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 2 / (far - near);
    m[11] = 0.0f;
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
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
    const GLchar* vertexShaderSource = readTextFile("tutorial03.vert");
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);
	checkShaderCompileStatus(vertexShaderId);

    const GLchar* fragmentShaderSource = readTextFile("tutorial03.frag");
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
    glGenBuffers(1, &trianglesId);
    glBindBuffer(GL_ARRAY_BUFFER, trianglesId);
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
    // we keep track of the aspect ratio to adjust the projection volume
    aspectRatio = 1.0f * width / height;
}

void render() {

    if (initialized == false) {
        createProgram();
        createTriangle();
        createQuad();
        initialized = true;
    }

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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
