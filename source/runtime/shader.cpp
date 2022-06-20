// 20 june 2022
// SB Engine v0.4.0-alpha

#include <runtime/shader.hpp>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>

Shader::Shader()
{
}

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
	compile(vertexPath, fragmentPath, geometryPath);
}

void Shader::compile(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
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
		std::ofstream logfile("latest.log", std::fstream::app);
		if (logfile.is_open())
		{
			logfile << "\nFailed to open shader files. Paths:\n" <<
			vertexPath << '\n' <<
			fragmentPath << '\n';
			if (geometryPath != nullptr)
				logfile << geometryPath << '\n';
			logfile << "Error log: " << e.what();
			logfile.close();
		}
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	GLuint s_vertex, s_fragment, s_geometry;
	
	s_vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(s_vertex, 1, &vShaderCode, NULL);
	glCompileShader(s_vertex);
	checkCompileErrors(s_vertex, ShaderError::Vertex);

	s_fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(s_fragment, 1, &fShaderCode, NULL);
	glCompileShader(s_fragment);
	checkCompileErrors(s_fragment, ShaderError::Fragment);

	if (geometryPath != nullptr)
	{
		const char* gShaderCode = geometryCode.c_str();
		s_geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(s_geometry, 1, &gShaderCode, NULL);
		glCompileShader(s_geometry);
		checkCompileErrors(s_geometry, ShaderError::Geometry);
	}
	
	program_id = glCreateProgram();
	glAttachShader(program_id, s_vertex);
	glAttachShader(program_id, s_fragment);
	
	if (geometryPath != nullptr)
		glAttachShader(program_id, s_geometry);
	
	glLinkProgram(program_id);
	checkCompileErrors(program_id, ShaderError::Program);
	
	/* Flag shaders for deletion. They will not be deleted
		until they are detached from program. */
	glDeleteShader(s_vertex);
	glDeleteShader(s_fragment);
	
	if (geometryPath != nullptr)
		glDeleteShader(s_geometry);
}

void Shader::terminate()
{
	GLuint current;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&current);
	if (program_id == current)
		glUseProgram(0);
	
	// Detach all shaders and terminate program
	glDeleteProgram(program_id);
}

void Shader::use() {
	glUseProgram(program_id);
}

void Shader::setInt(const char* name, int value) {
	glUniform1i(glGetUniformLocation(program_id, name), value);
}

void Shader::setFloat(const char* name, float value) {
	glUniform1f(glGetUniformLocation(program_id, name), value);
}

void Shader::setVec2(const char* name, const glm::vec2& value) {
	glUniform2fv(glGetUniformLocation(program_id, name), 1, &value[0]);
}

void Shader::setVec3(const char* name, const glm::vec3& value) {
	glUniform3fv(glGetUniformLocation(program_id, name), 1, &value[0]);
}

void Shader::setVec4(const char* name, const glm::vec4& value) {
	glUniform4fv(glGetUniformLocation(program_id, name), 1, &value[0]);
}

void Shader::setMat2(const char* name, const glm::mat2& mat) {
	glUniformMatrix2fv(glGetUniformLocation(program_id, name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const char* name, const glm::mat3& mat) {
	glUniformMatrix3fv(glGetUniformLocation(program_id, name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const char* name, const glm::mat4& mat) {
	glUniformMatrix4fv(glGetUniformLocation(program_id, name), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(GLuint shader, ShaderError type)
{
	GLint success;
	if (type != ShaderError::Program)
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			
			GLchar infoLog[1024];
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::ofstream logfile("latest.log", std::fstream::app);
			if (logfile.is_open())
			{
				logfile << "\nFailed to compile ";
				if (type == ShaderError::Vertex)
					logfile << "vertex ";
				else if (type == ShaderError::Fragment)
					logfile << "fragment ";
				else if (type == ShaderError::Geometry)
					logfile << "geometry ";
				logfile << "shader.\n";
				
				logfile << "InfoLog:\n" << infoLog;
				logfile.close();
			}
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[1024];
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::ofstream logfile("latest.log", std::fstream::app);
			if (logfile.is_open())
			{
				logfile << "\nShader program failed to link\nInfoLog:\n" << infoLog;
				logfile.close();
			}
		}
	}
}