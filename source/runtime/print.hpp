// 24 june 2022

#include <glm/gtc/type_ptr.hpp>
#include <string>

void print(const std::string& str);

void print(const glm::uvec2& v, const char end = '\n');

void print(const float value, const char end = '\n');

void print(const glm::vec2& v, const char end = '\n');

void print(const glm::vec3& v, const char end = '\n');

void print(const glm::vec4& v, const char end = '\n');

void print(const glm::mat3& m);

void print(const glm::mat4& m);

void print(const double value, const char end = '\n');

void print(const glm::dvec2& v, const char end = '\n');

void print(const glm::dvec3& v, const char end = '\n');

void print(const glm::dvec4& v, const char end = '\n');

void print(const glm::dmat3& m);

void print(const glm::dmat4& m);
