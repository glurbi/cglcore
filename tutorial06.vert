#version 330 core

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform vec3 color;
uniform vec3 lightDir;

in vec3 vPosition;
in vec3 vNormal;

out vec4 vColor;

void main(void) 
{ 
    /* We transform the normal in eye coordinates. */
    vec3 normalEye = vec3(mvMatrix * vec4(vNormal, 0.0f));
    
    /* We compute the dot product of the normal in eye coordinates by the light direction.
       The value will be positive when the diffuse light should be ignored, negative otherwise. */
    float dotProduct = dot(normalEye, lightDir);

    float diffuse = max(-dotProduct, 0.0f);
    
    vColor = vec4(color, 1.0f) * diffuse;
    gl_Position = mvpMatrix * vec4(vPosition, 1.0f);
}
