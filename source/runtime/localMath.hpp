// 27 june 2022
// SB Engine v0.1.0-alpha

#ifndef LOCAL_MATH_HPP
#define LOCAL_MATH_HPP

#include <common.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace lcm
{
	double getDist();
	double cosvec3(glm::dvec3 const&, glm::dvec3 const&);
	glm::dvec3 rotate(glm::dvec3 const& point, const double angle, glm::dvec3 const& r_axis);
	bool elenvec3(glm::dvec3 const& v, const double len);
	// glm::dmat4 decomposeRotation
	// (
	// 	const glm::dvec3& k, const glm::dvec3& v, // Before rotating
	// 	const glm::dvec3& ck, const glm::dvec3& cv // After
	// );
	glm::dvec3 gravitacc(glm::dvec3 const& falling, glm::dvec3 const& dest, const double mass);
	
	namespace sp
	{
		float getDist();
		float cosvec3(glm::vec3 const& v, glm::vec3 const& av);
		glm::vec3 rotate(glm::vec3 const& point, const float angle, glm::vec3 const& r_axis);
		bool elenvec3(glm::vec3 const& v, const float len);
		// glm::mat4 decomposeRotation
		// (
		// 	const glm::vec3& k, const glm::vec3& v, // Before rotating
		// 	const glm::vec3& ck, const glm::vec3& cv // After
		// );
		glm::vec3 gravitacc(glm::vec3 const& falling, glm::vec3 const& dest, const float mass);
	}
}

#endif