#version 330 core

uniform sampler2D textureDay;
uniform sampler2D textureNight;

smooth in float dotProduct;
smooth in vec2 texcoord;

out vec4 fColor;

void main(void) 
{
    float f = dotProduct / 2 + 0.5f;
    vec4 texColorDay = texture2D(textureDay, texcoord) * f;
    vec4 texColorNight = texture2D(textureNight, texcoord) * (1 - f);
    fColor = texColorDay + texColorNight;
}
