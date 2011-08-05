#include <stdio.h>
#include <glutils.h>

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

// Setting the swap interval is unfortunately platform dependent...
void setSwapInterval(int interval) {
#ifdef _WIN32
    if (wglewIsSupported("WGL_EXT_swap_control")) {
        wglSwapIntervalEXT(interval);
        printf("WGL_EXT_swap_control is supported.");
    }
#endif
}

