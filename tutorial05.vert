#version 330 core

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform vec3 color;
uniform sampler2D texture;
uniform vec3 lightDir;

in vec3 pos;
in vec3 normal;
in vec2 texcoord;

out vec3 vColor;
out vec2 vTexCoord;

void main(void) 
{ 
    /* We transform the normal in eye coordinates. */
    vec3 normalEye = vec3(mvMatrix * vec4(normal, 0.0f));
    
    /* We compute the dot product of the normal in eye coordinates by the light direction.
       The value will be positive when the diffuse light should be ignored, negative otherwise. */
    float dotProduct = dot(normalEye, lightDir);

    gl_Position = mvpMatrix * vec4(pos, 1.0f);
    vColor = vec3(0.0f, 1.0f, 1.0f) - (-min(dotProduct, 0.0f) * vec4(color, 1.0f)).rgb;

    vTexCoord = texcoord;
}
