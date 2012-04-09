#version 330 core

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform vec4 color;
uniform vec4 ambient;
uniform vec3 lightDir;
uniform sampler2D textureEarth;
uniform sampler2D textureCloud;

in vec3 vPosition;
in vec2 vTexCoord;

smooth out vec2 texcoord;

void main(void) 
{
    texcoord = vTexCoord;
    gl_Position = mvpMatrix * vec4(vPosition, 1.0f);
}
