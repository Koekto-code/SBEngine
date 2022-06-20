// 20 june 2022
// SB Engine v0.4.0-alpha
// Model loader specialized for .obj files

#ifndef MODEL_HPP
#define MODEL_HPP

#include <common.hpp>

#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>

#include <runtime/mesh.hpp>
#include <runtime/shader.hpp>

#include <string>
#include <vector>

class Model 
{
public:
	std::vector<Mesh> meshes;
	std::string directory; // path to model object

	Model();
	Model(const std::string& path);
	
	void Import(const std::string& path);
	void Render(Shader& shader);

private:
	void processNode(const struct aiNode *node, const struct aiScene *scene);
	Mesh convertMesh(const struct aiMesh *mesh, const struct aiScene *scene);
	void loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::vector<Texture>& loaded);
};

#endif