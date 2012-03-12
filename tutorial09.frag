#version 330 core

uniform sampler2D texture;

flat in vec4 vColor;
in vec2 texcoord;

out vec4 fColor;

void main(void) 
{ 
    vec4 texColor = texture2D(texture, texcoord);
    fColor = texColor;
	//fColor = vColor;
}
