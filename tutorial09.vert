#version 330 core

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform vec4 color;
uniform vec4 ambient;
uniform vec3 lightDir;
uniform sampler2D textureDay;
uniform sampler2D textureNight;

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

smooth out float dotProduct;
smooth out vec2 texcoord;

void main(void) 
{ 
    vec3 normalEye = vec3(mvMatrix * vec4(vNormal, 0.0f));
    dotProduct = dot(normalEye, lightDir);
    
    texcoord = vTexCoord;
    gl_Position = mvpMatrix * vec4(vPosition, 1.0f);
}
