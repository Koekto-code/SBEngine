// 27 june 2022

#include "print.hpp"
#include <iostream>

void print(const std::string& str)
{
	std::cout << str;
}

void print(const glm::uvec2& v, const char end)
{
	std::cout << "vec2(" << v.x << ", " << v.y << ')' << end;
}

void print(const float value, const char end)
{
	std::cout << value << end;
}

void print(const glm::vec2& v, const char end)
{
	std::cout << "vec2(" << v.x << ", " << v.y << ')' << end;
}

void print(const glm::vec3& v, const char end)
{
	std::cout << "vec3(" << v.x << ", " << v.y << ", " << v.z << ')' << end;
}

void print(const glm::vec4& v, const char end)
{
	printf("vec4(%7f, %7f, %7f, %7f)", v.x, v.y, v.z, v.w);
	std::cout << end;
}

void print(const glm::mat3& m)
{
	std::cout << "mat3(\n";
	for (uint8_t row = 0; row < 3; ++row) {
		printf("\t[%4.3f, %4.3f, %4.3f]\n", m[row][0], m[row][1], m[row][2]);
	}
	std::cout << ")\n";
}

void print(const glm::mat4& m)
{
	std::cout << "mat4(\n";
	for (uint8_t row = 0; row < 4; ++row) {
		printf("\t[%4.3f, %4.3f, %4.3f, %4.3f]\n", m[row][0], m[row][1], m[row][2], m[row][3]);
	}
	std::cout << ")\n";
}

void print(const double value, const char end)
{
	std::cout << value << end;
}

void print(const glm::dvec2& v, const char end)
{
	std::cout << "dvec2(" << v.x << ", " << v.y << ')' << end;
}

void print(const glm::dvec3& v, const char end)
{
	std::cout << "dvec3(" << v.x << ", " << v.y << ", " << v.z << ')' << end;
}

void print(const glm::dvec4& v, const char end)
{
	printf("vec4(%7f, %7f, %7f, %7f)", v.x, v.y, v.z, v.w);
	std::cout << end;
}

void print(const glm::dmat3& m)
{
	std::cout << "dmat3(\n";
	for (uint8_t row = 0; row < 3; ++row) {
		printf("\t[%4.3f, %4.3f, %4.3f]\n", m[row][0], m[row][1], m[row][2]);
	}
	std::cout << ")\n";
}

void print(const glm::dmat4& m)
{
	std::cout << "dmat4(\n";
	for (uint8_t row = 0; row < 4; ++row) {
		printf("\t[%4.3f, %4.3f, %4.3f, %4.3f]\n", m[row][0], m[row][1], m[row][2], m[row][3]);
	}
	std::cout << ")\n";
}
