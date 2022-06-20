// 20 june 2022
// SB Engine 0.4.0 alpha

#ifndef MESH_HPP
#define MESH_HPP

#include <common.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/gl.h>
#include <assimp/material.h>

#include <runtime/shader.hpp>

#include <string>
#include <vector>

struct Vertex
{
	glm::vec3 Position;
	glm::vec2 TexCoords;
	glm::vec3 Normal;
};

struct Texture
{
	GLuint id;
	aiTextureType type;
	char* pName;
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> triangles;
	std::vector<GLuint> edges; // edges are not displayed, but used as struts
	std::vector<Texture> textures;
	GLuint VAO, VBO, EBO;
	
	Mesh();
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> triangles, std::vector<GLuint> edges, std::vector<Texture> textures);
	
	void Init(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<GLuint> edges, std::vector<Texture> textures);
	void Render(Shader& shader);
	
protected:
	void applyTextures(Shader& shader);
	void Setup();
};

#endif