#version 330 core

uniform sampler2D texture;

in vec4 vColor;
in vec2 vTexCoord;

out vec4 fColor;

void main(void) 
{ 
	fColor = texture2D(texture, vTexCoord);
}
