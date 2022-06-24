// 24 june 2022
// SB Engine v0.1.0-alpha
// Model loader specialized for .obj files

#include <runtime/model.hpp>
#include <runtime/glUtil.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/gl.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <fstream>
#include <map>


Model::Model()
{
}

Model::Model(const std::string& path)
{
	Import(path);
}

void Model::Render(Shader& shader)
{ 
	for (unsigned i = 0; i < meshes.size(); ++i)
		meshes[i].Render(shader);
}

void Model::Import(const std::string& path)
{
	std::ofstream logfile;
	
	// Check if program can access model file
	std::ifstream fin(path);
	if (fin.is_open())
		fin.close();
	else
	{
		logfile.open("latest.log", std::fstream::app);
		if (logfile.is_open())
		{
			logfile << "Failed to open file: " << path << '\n';
			logfile.close();
		}
		exit(-20001);
	}
	
	Assimp::Importer importer;
	const struct aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		logfile.open("latest.log", std::fstream::app);
		if (logfile.is_open())
		{
			logfile << "--Assimp: error: " << importer.GetErrorString() << '\n';
			logfile.close();
		}
		exit(-20002);
	}
	
	size_t pos = path.find_last_of("\\/");
	directory = (std::string::npos == pos) ? "" : path.substr(0, pos+1);

	processNode(scene->mRootNode, scene);
}

void Model::processNode(const struct aiNode *node, const struct aiScene *scene)
{
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
		meshes.push_back(convertMesh(scene->mMeshes[node->mMeshes[i]], scene));

	for(unsigned int i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}

Mesh Model::convertMesh(const struct aiMesh *mesh, const struct aiScene *scene)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> triangles;
	std::vector<GLuint> edges;
	std::vector<Texture> textures;

	vertices.resize(mesh->mNumVertices);
	for (unsigned int i = 0; i < vertices.size(); ++i)
	{
		Vertex vertex;
		vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		
		// A vertex may contain up to 8 tex coords. We use only first one.
		if (mesh->mTextureCoords[0])
			vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		else vertex.TexCoords = glm::vec2(0.0f);
		
		vertices[i] = vertex;
	}
	
	// Iterating triangles
	for (unsigned long i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace &face = mesh->mFaces[i];
		
		if (face.mNumIndices == 3)
		{
			for (unsigned char j = 0; j < face.mNumIndices; ++j)
				triangles.push_back(face.mIndices[j]);
		}
		else if (face.mNumIndices == 2)
		{
			edges.push_back(face.mIndices[0]);
			edges.push_back(face.mIndices[1]);
		}
	}
	
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	for (
		aiTextureType textype = aiTextureType_DIFFUSE;
		textype <= aiTextureType_SHININESS;
		textype = (aiTextureType)(textype + 1)
	) loadMaterialTextures(material, textype, textures);
	
	return Mesh(vertices, triangles, edges, textures);
}

void Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::vector<Texture>& dest)
{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
	{
		aiString texname;
		mat->GetTexture(type, i, &texname);
		
		for (auto tex: dest) {
			if (!std::strcmp(tex.pName, texname.C_Str())) {
				dest.push_back(tex);
				goto next;
			}
		}

		Texture texture;
		loadTexture((this->directory + "/" + texname.C_Str()).c_str(), &texture.id);

		texture.pName = new char[strlen(texname.C_Str()) + 1];
		strcpy(texture.pName, texname.C_Str());
		texture.type = type;
		dest.push_back(texture);

		next: continue;
	}
}