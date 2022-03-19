// 19 mar 2022

#ifndef MISC_HPP
#define MISC_HPP

#include <glm/gtc/type_ptr.hpp>

static float max(float x, float y)
{
	return x > y ? x : y;
}

static double max(double x, double y)
{
	return x > y ? x : y;
}

static void writeVector(std::ofstream& file, glm::dvec3 v)
{
	file << "dvec3(" << v.x << ", " << v.y << ", " << v.z << ")\n";
}

static void writeVector(std::ofstream& file, glm::vec3 v)
{
	file << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")\n";
}

#endif
