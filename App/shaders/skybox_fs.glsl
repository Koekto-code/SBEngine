#version 330 core

in vec3 wTexCoords;
out vec4 FragColor;

uniform samplerCube skyTex;

void main()
{    
    FragColor = texture(skyTex, wTexCoords);
}