#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/glew.h>
#include <GL/glxew.h>
#include <vector>
#include <stack>
#include <string>

/*
 * In this tutorial, we render a rotating sphere lighted with ambient
 * and diffuse light component, using gouraud lighting and flat shading.
 */

// C/C++ does not have a default definition for pi!
const float pi = atan(1.0f) * 4.0f;

inline float toRadians(float degrees) {
    return degrees * pi / 180.0f;
}

class vector2 {
public:
    vector2(float x, float y): x(x), y(y) {}
    const float x, y;
};

class vector3 {
public:
    vector3(float x, float y, float z): x(x), y(y), z(z) {}
    void dump(float** p) { (*p)[0] = x; (*p)[1] = y; (*p)[2] = z; *p += 3; }
    vector3 normalize() {
        float norm = sqrt(x*x + y*y + z*z);
        return vector3(x / norm, y / norm, z / norm);
    }
    const float x, y, z;
};

vector3 midPoint(vector3 p1, vector3 p2) {
    return vector3((p1.x + p2.x) / 2, (p1.y + p2.y) / 2, (p1.z + p2.z) / 2);
}

inline long currentTimeMillis() {
	return clock() / (CLOCKS_PER_SEC / 1000);
}

class matrix44 {
public:
	matrix44 multm(const matrix44& m2) {
		matrix44 m;
		for (int i = 0; i < 4; i++) {
		    for (int j = 0; j < 4; j++) {
		        m.f[i+j*4] =
		            f[i+0] * m2.f[j*4+0] +
		            f[i+4] * m2.f[j*4+1] +
		            f[i+8] * m2.f[j*4+2] +
		            f[i+12] * m2.f[j*4+3];
		    }
		}
		return m;
	}
	float f[16];
};

class triangle {
public:
    triangle(vector3 p1, vector3 p2, vector3 p3): p1(p1), p2(p2), p3(p3) {}
    void dump(float** p) { p1.dump(p); p2.dump(p); p3.dump(p); }
    vector3 center() { return vector3((p1.x+p2.x+p3.x)/3, (p1.y+p2.y+p3.y)/3, (p1.z+p2.z+p3.z)/3); }
    vector3 p1, p2, p3;
};

matrix44 identity() {
    matrix44 identityMatrix;
    float* mi = identityMatrix.f;
    mi[0] = 1.0f;
    mi[1] = 0.0f;
    mi[2] = 0.0f;
    mi[3] = 0.0f;
    mi[4] = 0.0f;
    mi[5] = 1.0f;
    mi[6] = 0.0f;
    mi[7] = 0.0f;
    mi[8] = 0.0f;
    mi[9] = 0.0f;
    mi[10] = 1.0;
    mi[11] = 0.0f;
    mi[12] = 0.0f;
    mi[13] = 0.0f;
    mi[14] = 0.0;
    mi[15] = 1.0f;
    return identityMatrix;
}

matrix44 frustum(float left, float right, float bottom, float top, float near, float far) {
    matrix44 frustumMatrix;
    float* m = frustumMatrix.f;
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
    return frustumMatrix;
}

matrix44 translate(float x, float y, float z) {
	matrix44 translateMatrix;
	float* m = translateMatrix.f;
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
	return translateMatrix;
}

matrix44 rotate(float a, float x, float y, float z) {
	matrix44 rotateMatrix;
	float* m = rotateMatrix.f;
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
	return rotateMatrix;
}

class mstack {
public:
    mstack() {
        s.push(identity());
    }
    void push(matrix44 m) {
        s.push(s.top().multm(m));
    }
    matrix44 top() {
        return s.top();
    }
    std::stack<matrix44> s;
};

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

// Up to 16 attributes per vertex is allowed so any value between 0 and 15 will do.
const int POSITION_ATTRIBUTE_INDEX = 0;
const int NORMAL_ATTRIBUTE_INDEX = 1;
const int TEXCOORD_ATTRIBUTE_INDEX = 2;

// a class for managing a texture
class Texture {

public:

    Texture(const std::string& s) {
        imageFile = s;
    }

    void init() {
        SDL_Surface *image = IMG_Load(imageFile.c_str());
        if(!image) {
            printf("IMG_Load: %s\n", IMG_GetError());
        }
        int width = image->w;
        int height = image->h;
        SDL_Surface* rgbaImage = SDL_CreateRGBSurface(0, width, height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        SDL_BlitSurface(image, 0, rgbaImage, 0);
        GLenum format = GL_RGBA;
        
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, rgbaImage->pixels);
    }

    GLuint getId() {
        return textureId;
    }

private:

    std::string imageFile;
    GLuint textureId;
};

// a class for calculating the vertices and drawing a sphere
class Sphere {

public:
    
    void init() {
        float* positions;
        float* normals;
        float* texcoords;
        
        int psize = sphereAttributeCount(depth)*3*sizeof(float);
        int nsize = sphereAttributeCount(depth)*3*sizeof(float);
        int tsize = sphereAttributeCount(depth)*2*sizeof(float);
        
        positions = (float*) malloc(psize);
        normals = (float*) malloc(nsize);
        texcoords = (float*) malloc(tsize);
        
        createSphereAttributes(positions, normals, texcoords);
        
        glGenBuffers(1, &spherePositionsId);
        glBindBuffer(GL_ARRAY_BUFFER, spherePositionsId);
        glBufferData(GL_ARRAY_BUFFER, psize, positions, GL_STATIC_DRAW);

        glGenBuffers(1, &sphereNormalsId);
        glBindBuffer(GL_ARRAY_BUFFER, sphereNormalsId);
        glBufferData(GL_ARRAY_BUFFER, nsize, normals, GL_STATIC_DRAW);
        
        glGenBuffers(1, &sphereTexCoordsId);
        glBindBuffer(GL_ARRAY_BUFFER, sphereTexCoordsId);
        glBufferData(GL_ARRAY_BUFFER, tsize, texcoords, GL_STATIC_DRAW);

        free(positions);
        free(normals);
        free(texcoords);
    }
    
    void render() {
        glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
        glBindBuffer(GL_ARRAY_BUFFER, spherePositionsId);
        glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
        glBindBuffer(GL_ARRAY_BUFFER, sphereNormalsId);
        glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(TEXCOORD_ATTRIBUTE_INDEX);
        glBindBuffer(GL_ARRAY_BUFFER, sphereTexCoordsId);
        glVertexAttribPointer(TEXCOORD_ATTRIBUTE_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);        
        glDrawArrays(GL_TRIANGLES, 0, sphereAttributeCount(depth));
        glDisableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);
        glDisableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
        glDisableVertexAttribArray(TEXCOORD_ATTRIBUTE_INDEX);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
private:

    GLuint spherePositionsId;
    GLuint sphereNormalsId;
    GLuint sphereTexCoordsId;
    
    static const int depth = 4;

    inline int sphereAttributeCount(int n) { return 8 * pow(4, n) * 3; }

    inline vector2 cart2geog(vector3 p) { return vector2(atan2(p.y, p.x), asin(p.z)); }

    void texCoord(float** t, vector3 p, triangle tr) {
        vector2 geog = cart2geog(p);
        vector2 geogtr = cart2geog(tr.center());
        float lat = geog.y;
        float lon = geog.x;
        float t1 = lon / (2.0f*pi) + 0.5f;
        float t2 = -1.0f * lat / pi + 0.5f;
        if (t1 == 1.0f && geogtr.x < 0.5f) { t1 = 0.0f; }
        if (t1 == 0.0f && geogtr.x > 0.5f) { t1 = 1.0f; }       
        **t = t1;
        (*t)++;
        **t = t2;
        (*t)++;
    }

    void refine(int d, triangle tr, float** p, float** n, float** t) {
        if (d == depth) {
            tr.dump(p);
            tr.dump(n);
            texCoord(t, tr.p1, tr);
            texCoord(t, tr.p2, tr);
            texCoord(t, tr.p3, tr);
        } else {
            vector3 m1 = midPoint(tr.p2, tr.p3).normalize();
            vector3 m2 = midPoint(tr.p3, tr.p1).normalize();
            vector3 m3 = midPoint(tr.p1, tr.p2).normalize();
            refine(d + 1, triangle(tr.p1, m3, m2), p, n, t);
            refine(d + 1, triangle(m3, tr.p2, m1), p, n, t);
            refine(d + 1, triangle(m1, m2, m3), p, n, t);
            refine(d + 1, triangle(m2, m1, tr.p3), p, n, t);
        }
    }

    void createSphereAttributes(float* p, float* n, float* t) {
        //
        // we refine each side of an octahedron
        // cf http://paulbourke.net/miscellaneous/sphere_cylinder/
        //
        refine(0, triangle(vector3(0.0f, 1.0f, 0.0f), vector3(0.0f, 0.0f, 1.0f), vector3(1.0f, 0.0f, 0.0f)), &p, &n, &t);
        refine(0, triangle(vector3(0.0f, 1.0f, 0.0f), vector3(1.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, -1.0f)), &p, &n, &t);
        refine(0, triangle(vector3(0.0f, 1.0f, 0.0f), vector3(0.0f, 0.0f, -1.0f), vector3(-1.0f, 0.0f, 0.0f)), &p, &n, &t);
        refine(0, triangle(vector3(0.0f, 1.0f, 0.0f), vector3(-1.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, 1.0f)), &p, &n, &t);
        refine(0, triangle(vector3(0.0f, -1.0f, 0.0f), vector3(1.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, 1.0f)), &p, &n, &t);
        refine(0, triangle(vector3(0.0f, -1.0f, 0.0f), vector3(0.0f, 0.0f, 1.0f), vector3(-1.0f, 0.0f, 0.0f)), &p, &n, &t);
        refine(0, triangle(vector3(0.0f, -1.0f, 0.0f), vector3(-1.0f, 0.0f, 0.0f), vector3(0.0f, 0.0f, -1.0f)), &p, &n, &t);
        refine(0, triangle(vector3(0.0f, -1.0f, 0.0f), vector3(0.0f, 0.0f, -1.0f), vector3(1.0f, 0.0f, 0.0f)), &p, &n, &t);
    }

};

// defines the perspective projection volume
const float left = -1.0f;
const float right = 1.0f;
const float bottom = -1.0f;
const float top = 1.0f;
const float nearPlane = 2.0f;
const float farPlane = 10.0f;

bool initialized = false;
long startTimeMillis;
GLuint programId;
Texture textureDay("earth_day.jpg");
Texture textureNight("earth_night.jpg");
Sphere sphere;

float aspectRatio;
int frameCount;
int totalFrameCount;
int currentWidth;
int currentHeight;

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
    const GLchar* vertexShaderSource = readTextFile("tutorial09.vert");
    int vertexShaderSourceLength = strlen(vertexShaderSource);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShaderSource, &vertexShaderSourceLength);
    glCompileShader(vertexShaderId);
	checkShaderCompileStatus(vertexShaderId);

    const GLchar* fragmentShaderSource = readTextFile("tutorial09.frag");
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

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    // we keep track of the aspect ratio to adjust the projection volume
    aspectRatio = 1.0f * width / height;
    currentWidth = width;
    currentHeight = height;
}

// thanks to http://openglbook.com/the-book/chapter-1-getting-started/#toc-measuring-performance
void timer(int value) {
    char title[512];
    sprintf(title, "Tutorial09: %d FPS @ %d x %d", frameCount * 4, currentWidth, currentHeight);
    SDL_WM_SetCaption(title, title);
    frameCount = 0;
}

void render() {

    if (initialized == false) {
        glEnable(GL_TEXTURE_2D);    
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        sphere.init();
        textureDay.init();
        textureNight.init();
        createProgram();
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
    mstack mvp;
    mstack mv;
    
    matrix44 frustumMat = frustum(left, right, bottom / aspectRatio, top / aspectRatio, nearPlane, farPlane);
    matrix44 translateMat = translate(0.0f, 0.0f, -3.0f);
    matrix44 rotateMat1 = rotate(-90, 1.0f, 0.0f, 0.0f);
    matrix44 rotateMat2 = rotate(-90, 0.0f, 0.0f, 1.0f);
    matrix44 rotateMat3 = rotate(1.0f * elapsed / 50, 0.0f, 0.0f, 1.0f);

    mvp.push(frustumMat);
    mvp.push(translateMat);
    mvp.push(rotateMat1);
    mvp.push(rotateMat2);
    mvp.push(rotateMat3);

    mv.push(translateMat);
    mv.push(rotateMat1);
    mv.push(rotateMat2);
    mv.push(rotateMat3);
    
    // activate the textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureDay.getId());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureNight.getId());
    
    // set the uniforms before rendering
    GLuint mvpMatrixUniform = glGetUniformLocation(programId, "mvpMatrix");
    GLuint mvMatrixUniform = glGetUniformLocation(programId, "mvMatrix");
    GLuint textureDayUniform = glGetUniformLocation(programId, "textureDay");
    GLuint textureNightUniform = glGetUniformLocation(programId, "textureNight");
    GLuint ambientUniform = glGetUniformLocation(programId, "ambient");
    GLuint lightDirUniform = glGetUniformLocation(programId, "lightDir");
    glUniformMatrix4fv(mvpMatrixUniform, 1, false, mvp.top().f);
    glUniformMatrix4fv(mvMatrixUniform, 1, false, mv.top().f);
    glUniform3f(lightDirUniform, 1.0f, 0.0f, -0.5f);
    glUniform4f(ambientUniform, 0.1f, 0.1f, 0.1f, 1.0f);
    glUniform1i(textureDayUniform, 0);
    glUniform1i(textureNightUniform, 1);

    // render!
    sphere.render();

    // display rendering buffer
    SDL_GL_SwapBuffers();
}

int main(int argc, char **argv) {

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);
    SDL_Surface* surfDisplay = SDL_SetVideoMode(900, 900, 32,
            SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);

    // must be called AFTER the OpenGL context has been created
    glewInit();
    reshape(900, 900);

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                event.type == SDL_KEYDOWN ||
                event.type == SDL_KEYUP)
            {
                done = true;
            }
        }
        render();
    }

    SDL_FreeSurface(surfDisplay);
    SDL_Quit();
    return 0;
}
