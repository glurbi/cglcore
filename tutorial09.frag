#version 330 core

uniform sampler2D textureDay;
uniform sampler2D textureNight;

smooth in float dotProduct;
smooth in vec2 texcoord;

out vec4 fColor;

void main(void) 
{
    float fDay = clamp(dotProduct + 0.9f, 0.0f, 1.0f);
    float fNight = clamp(abs(dotProduct - 0.9f), 0.0f, 1.0f);
    vec4 texColorDay = texture2D(textureDay, texcoord);
    vec4 texColorNight = texture2D(textureNight, texcoord);
    fColor = texColorDay * fDay + texColorNight * fNight;
}
