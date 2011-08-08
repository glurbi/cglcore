#version 330 core

uniform sampler2D texture;

in vec3 vColor;
in vec2 vTexCoord;

out vec4 fColor;

void main(void) 
{
    vec4 texColor = texture2D(texture, vTexCoord);
    if (texColor.a == 0.0f) {
	    fColor = vec4(vColor, 0.7f);
    } else {
        fColor = texColor;
    }
}
