#version 330 core

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform vec4 color;
uniform vec4 ambient;
uniform vec3 lightDir;

in vec3 vPosition;
in vec3 vNormal;

smooth out vec3 normalEye;

void main(void) 
{ 
    /* We transform the normal in eye coordinates. */
    normalEye = vec3(mvMatrix * vec4(vNormal, 0.0f));
    
    gl_Position = mvpMatrix * vec4(vPosition, 1.0f);
}
