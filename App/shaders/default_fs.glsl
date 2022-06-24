#version 330 core

in vec2 wTexCoords;
in vec3 wNormal;
in vec3 wFragPos;

out vec4 FragColor;
 
uniform sampler2D tex_d0;
uniform vec3 viewPos;

void main()
{
	vec3 result = vec3(texture(tex_d0, wTexCoords));
	FragColor = vec4(result, 1.0);
}