#version 330 core

uniform mat4 mvpMatrix;
uniform sampler2D textureEarth;
uniform sampler2D textureCloud;
uniform float threshold;

in vec3 vPosition;
in vec2 vTexCoord;

smooth out vec2 texcoord;

void main(void) 
{
    texcoord = vTexCoord;
    gl_Position = mvpMatrix * vec4(vPosition, 1.0f);
}
