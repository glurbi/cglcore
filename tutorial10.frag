#version 330 core

uniform sampler2D textureEarth;
uniform sampler2D textureCloud;
uniform float threshold;

smooth in vec2 texcoord;

out vec4 fColor;

void main(void) 
{
    vec4 texColorCloud = texture2D(textureCloud, texcoord);
    if (texColorCloud.r < threshold) {
        discard;
    }
        
    vec4 texColorEarth = texture2D(textureEarth, texcoord);
    fColor = texColorEarth;
}

