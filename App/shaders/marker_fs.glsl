#version 330 core

out vec4 FragColor;

uniform vec4 fragColor;

void main()
{
	FragColor = fragColor;
}