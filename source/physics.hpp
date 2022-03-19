// 13 mar 2022

#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace ph
{
	double getDist();
	double cosvec3(glm::dvec3, glm::dvec3);
	glm::dvec3 rotate(glm::dvec3, glm::dvec3, double);
	int elenvec3(glm::dvec3, double); // as bool
	glm::dvec3 gravitacc(glm::dvec3, glm::dvec3, double);
	
	namespace sp
	{
		float getDist();
		float cosvec3(glm::vec3, glm::vec3);
		glm::vec3 rotate(glm::vec3, glm::vec3, float);
		int elenvec3(glm::vec3, float); // as bool
		glm::vec3 gravitacc(glm::vec3, glm::vec3, float);
	}
}

#endif
