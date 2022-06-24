#version 330 core
layout (location = 0) in vec3 vPosition;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	gl_Position = projection * view * vec4(vPosition, 1.0);
}