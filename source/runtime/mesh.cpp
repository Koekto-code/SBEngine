// 24 june 2022
// SB Engine v0.1.0-alpha

#include <runtime/mesh.hpp>
#include <runtime/print.hpp>
#include <cstdlib>

Mesh::Mesh()
{
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> triangles, std::vector<GLuint> edges, std::vector<Texture> textures)
{
	Init(vertices, triangles, edges, textures);
}

void Mesh::Init(std::vector<Vertex> vertices, std::vector<GLuint> triangles, std::vector<GLuint> edges, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->triangles = triangles;
	this->edges = edges;
	this->textures = textures;

	Setup();
}

void Mesh::applyTextures(Shader& shader)
{
	if (!textures.size()) return;
	
	unsigned char loadedTypes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	char texname[8];
	std::memcpy(texname, "tex_", 4);
	
	GLint glmctiu;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &glmctiu);
	for (unsigned long i = 0; i < (unsigned long)glmctiu && i < textures.size(); ++i)
	{
		unsigned char textype;
		unsigned char texnum = loadedTypes[textures[i].type]++;
		
		switch(textures[i].type)
		{
			case aiTextureType_DIFFUSE:
				textype = 'd';
				break;
			case aiTextureType_SPECULAR:
				textype = 's';
				break;
			case aiTextureType_AMBIENT:
				textype = 'a';
				break;
			case aiTextureType_EMISSIVE:
				textype = 'e';
				break;
			case aiTextureType_HEIGHT:
				textype = 'h';
				break;
			case aiTextureType_NORMALS:
				textype = 'n';
				break;
			case aiTextureType_SHININESS:
				textype = 'i';
				break;
			default:
				textype = 'u'; // undefined
				break;
		}
		texname[4] = textype;
		
		// itoa(texnum, texname + 5, 10);
		texname[5] = '0' + texnum / 10;
		texname[6] = '0' + texnum % 10;
		
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
		shader.setInt(texname, i);
	}
}

void Mesh::Render(Shader& shader)
{
	applyTextures(shader);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_INT, 0);
	
	glBindVertexArray(0);
}

/* private */ void Mesh::Setup()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(GLuint), &triangles[0], GL_STATIC_DRAW);

	// layout (location = 0) in vec3 vPosition;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	glEnableVertexAttribArray(0);
	
	// layout (location = 1) in vec2 vTexCoords;
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(1);

	// layout (location = 2) in vec3 vNormal;
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}