// 18 mar 2022

#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
	unsigned int ID;
	void structShaders(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	// void Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	
	void use();

	void setBool(const char* name, bool value);
	void setInt(const char* name, int value);
	void setFloat(const char* name, float value);

	void setVec2(const char* name, const glm::vec2& value);
	// void setVec2(const char* name, float x, float y);
	void setVec3(const char* name, const glm::vec3& value);
	// void setVec3(const char* name, float x, float y, float z);
	void setVec4(const char* name, const glm::vec4& value);
	// void setVec4(const char* name, float x, float y, float z, float w);

	void setMat2(const char* name, const glm::mat2& mat);
	void setMat3(const char* name, const glm::mat3& mat);
	void setMat4(const char* name, const glm::mat4& mat);

private:
	void checkCompileErrors(GLuint shader, int type); // type: 0: Vertex; 1: Fragment; 2: Geometry; 3: Program
};

#endif
