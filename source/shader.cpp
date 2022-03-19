// 18 mar 2022

#include "shader.hpp"
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>

void Shader::structShaders(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::string geometryCode;
	std::ifstream gShaderFile;
	
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	if (geometryPath != nullptr)
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		
		vShaderFile.close();
		fShaderFile.close();
		
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		
		if (geometryPath != nullptr)
		{
		gShaderFile.open(geometryPath);
		std::stringstream gShaderStream;
		gShaderStream << gShaderFile.rdbuf();
		gShaderFile.close();
		geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure& e)
	{
		std::ofstream logfile;
		logfile.open("latest.log", std::fstream::app);
		logfile << "\nShader class: Failed to open shader files\nShader paths:\n" <<
		vertexPath << '\n' <<
		fragmentPath << '\n';
		if (geometryPath != nullptr)
		logfile << geometryPath << '\n';
		logfile.close();
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	unsigned int vertex, fragment;
	
	
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, 0);

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, 1);
   
	unsigned int geometry;
	if (geometryPath != nullptr)
	{
		const char* gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, 2);
	}
	
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	
	if (geometryPath != nullptr)
		glAttachShader(ID, geometry);
	
	glLinkProgram(ID);
	checkCompileErrors(ID, 3);
	
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	
	if (geometryPath != nullptr)
		glDeleteShader(geometry);
}

void Shader::use() {
	glUseProgram(ID);
}

void Shader::setBool(const char* name, bool value) {
	glUniform1i(glGetUniformLocation(ID, name), (int)value);
}
void Shader::setInt(const char* name, int value) {
	glUniform1i(glGetUniformLocation(ID, name), value);
}
void Shader::setFloat(const char* name, float value) {
	glUniform1f(glGetUniformLocation(ID, name), value);
}

void Shader::setVec2(const char* name, const glm::vec2& value) {
	glUniform2fv(glGetUniformLocation(ID, name), 1, &value[0]);
}
/* void Shader::setVec2(const char* name, float x, float y) {
	glUniform2f(glGetUniformLocation(ID, name), x, y);
} */
void Shader::setVec3(const char* name, const glm::vec3& value) {
	glUniform3fv(glGetUniformLocation(ID, name), 1, &value[0]);
}
/* void Shader::setVec3(const char* name, float x, float y, float z) {
	glUniform3f(glGetUniformLocation(ID, name), x, y, z);
} */
void Shader::setVec4(const char* name, const glm::vec4& value) {
	glUniform4fv(glGetUniformLocation(ID, name), 1, &value[0]);
}
/* void Shader::setVec4(const char* name, float x, float y, float z, float w) {
	glUniform4f(glGetUniformLocation(ID, name), x, y, z, w);
} */

void Shader::setMat2(const char* name, const glm::mat2& mat) {
	glUniformMatrix2fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat3(const char* name, const glm::mat3& mat) {
	glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
}
void Shader::setMat4(const char* name, const glm::mat4& mat) {
	glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(GLuint shader, int type)
{
	GLint success;
	if (type != 3)
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			
			GLchar infoLog[1024];
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::ofstream logfile;
			logfile.open("latest.log", std::fstream::app);
			logfile << "\nShader class: Compiling error of type ";
			if (type == 0)
				logfile << "VERTEX";
			else if (type == 1)
				logfile << "FRAGMENT";
			else logfile << "GEOMETRY";
			
			logfile << "\nInfoLog:\n" << infoLog;
			logfile.close();
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			std::ofstream logfile;
			GLchar infoLog[1024];
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			logfile.open("latest.log", std::fstream::app);
			logfile << "\nShader class: Program failed to link\nInfoLog:\n" << infoLog;
			logfile.close();
		}
	}
}