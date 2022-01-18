// 14 jan 2022

#ifndef MISC_HPP
#define MISC_HPP

#include <cstdio>
#include <glm/gtc/type_ptr.hpp>

void displayVector(glm::dvec3 v)
{
	printf("dvec3(%9.6f, %9.6f, %9.6f)\n", v.x, v.y, v.z);
}

void displayVector(glm::vec3 v)
{
	printf("vec3(%9.6f, %9.6f, %9.6f)\n", v.x, v.y, v.z);
}

#endif
