#version 330 core

uniform sampler2D textureDay;
uniform sampler2D textureNight;

flat in vec4 vColor;
in vec2 texcoord;

out vec4 fColor;

void main(void) 
{ 
    vec4 texColorDay = texture2D(textureDay, texcoord);
    vec4 texColorNight = texture2D(textureNight, texcoord);
    fColor = texColorDay / 2 + texColorNight / 2;
}
