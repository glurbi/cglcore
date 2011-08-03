#include <stdio.h>
#include <glutils.h>

void checkShaderStatus(GLuint shaderId) {
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

