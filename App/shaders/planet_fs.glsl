#version 330 core

in vec2 wTexCoords;
out vec4 FragColor;

uniform sampler2D worldmap;

void main()
{
	FragColor = texture(worldmap, wTexCoords);
}