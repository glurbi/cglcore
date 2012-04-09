#version 330 core

uniform sampler2D textureEarth;
uniform sampler2D textureCloud;

smooth in vec2 texcoord;

out vec4 fColor;

void main(void) 
{
    vec4 texColorEarth = texture2D(textureEarth, texcoord);
    vec4 texColorCloud = texture2D(textureCloud, texcoord);
    fColor = texColorEarth;
}

