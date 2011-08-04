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
	vec3 normalEye;
	float dotProduct;
	
	/* We transform the normal in eye coordinates. */
	normalEye = vec3(mvMatrix * vec4(vNormal, 0.0f));
	
	/* We compute the dot product of the normal in eye coordinates by the light direction.
       The value will be positive when the diffuse light should be ignored, negative otherwise. */
	dotProduct = dot(normalEye, lightDir);

	gl_Position = mvpMatrix * vec4(vPosition, 1.0f);
	vColor = -min(dotProduct, 0.0f) * vec4(color, 1.0f);
}
