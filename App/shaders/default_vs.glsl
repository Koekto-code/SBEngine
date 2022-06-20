#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 vTexCoords;
layout (location = 2) in vec3 vNormal;

out vec2 wTexCoords;
out vec3 wNormal;
out vec3 wFragPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	wTexCoords = vTexCoords;
	wNormal = vec3(mat3(model) * vNormal);
	wFragPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}