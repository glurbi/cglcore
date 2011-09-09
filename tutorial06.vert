#version 330 core

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform vec3 color;
uniform vec3 lightDir;

in vec3 vPosition;
out vec4 vColor;

void main(void) 
{ 
	gl_Position = mvpMatrix * vec4(vPosition, 1.0f);
	vColor = vec4(color, 1.0f);
}
