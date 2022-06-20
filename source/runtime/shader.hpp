// 20 june 2022
// SB Engine v0.4.0-alpha

#ifndef SHADER_HPP
#define SHADER_HPP

#include <common.hpp>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

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

	void setInt(const char* name, GLint value);
	void setFloat(const char* name, GLfloat value);

	void setVec2(const char* name, const glm::vec2& value);
	void setVec3(const char* name, const glm::vec3& value);
	void setVec4(const char* name, const glm::vec4& value);

	void setMat2(const char* name, const glm::mat2& mat);
	void setMat3(const char* name, const glm::mat3& mat);
	void setMat4(const char* name, const glm::mat4& mat);

private:
	GLuint program_id;
	void checkCompileErrors(GLuint shader, ShaderError type); // type: 0: Vertex; 1: Fragment; 2: Geometry; 3: Program
};

#endif