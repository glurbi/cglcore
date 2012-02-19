#version 330 core

uniform vec4 color;
uniform vec4 ambient;
uniform vec3 lightDir;

smooth in vec3 normalEye;

out vec4 fColor;

void main(void) 
{
    /* We compute the dot product of the normal in eye coordinates by the light direction.
       The value will be positive when the diffuse light should be ignored, negative otherwise. */
    float dotProduct = dot(normalEye, lightDir);
    vec4 diffuse = color * max(-dotProduct, 0.0f);

    /* We compute the reflection to get the specular component */
    vec3 reflection = normalize(reflect(lightDir, normalEye));
    float specFactor = pow(max(0.0f, dot(normalEye, reflection)), 64.0f);
    vec4 specular = specFactor * vec4(1.0f, 1.0f, 1.0f, 1.0f);
 
    fColor =  ambient + diffuse + specular;
}
