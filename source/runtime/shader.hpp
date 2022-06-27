// 27 june 2022
// SB Engine v0.1.0-alpha

#ifndef SHADER_HPP
#define SHADER_HPP

#include <common.hpp>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <map>

enum class ShaderError
{
	Vertex,
	Fragment,
	Geometry,
	Program
};

class Shader
{
public:
	Shader();
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

	void compile(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	void terminate();
	void use();
	GLint id();

	void setInt(const char* name, GLint value);
	void setFloat(const char* name, GLfloat value);

	void setVec2(const char* name, const glm::vec2& val);
	void setVec3(const char* name, const glm::vec3& val);
	void setVec4(const char* name, const glm::vec4& val);

	void setMat2(const char* name, const glm::mat2& val);
	void setMat3(const char* name, const glm::mat3& val);
	void setMat4(const char* name, const glm::mat4& val);
protected:
	std::map<const char*, GLint> uniformLocations;
private:
	GLuint programId;
	GLint sh_getUniformLocation(const GLchar* name);
	void checkCompileErrors(GLuint shader, ShaderError type);
};

#endif