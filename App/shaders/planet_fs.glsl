#version 330 core

in vec3 ourColor;
in vec2 wTexCoords;
out vec4 FragColor;

uniform sampler2D texture3;

void main()
{
	FragColor = texture(texture3, wTexCoords);
}