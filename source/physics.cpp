// 13 mar 2022

#include "physics.hpp"

namespace ph
{
	static const double GN(6.674301515e-11);
	// static const double C(299792458.0);

	static double sqrdist;

	double getDist() {
		return sqrt(sqrdist);
	}

	double cosvec3(glm::dvec3 v, glm::dvec3 av) {
		return glm::dot(v, av) / (glm::length(v) * glm::length(av));
	}

	glm::dvec3 rotate(glm::dvec3 /*normalized*/r_axis, glm::dvec3 point, double angle)
	{
		glm::dvec3 sub = point - r_axis * glm::length(point) * cosvec3(r_axis, point);
		return point
		+ glm::normalize(glm::cross(r_axis, sub)) * (sin(angle) * glm::length(sub))
		- sub * (1.0 - cos(angle));
	}
	
	int elenvec3(glm::dvec3 v, double len)
	{
		if (abs(v.x) > len || abs(v.y) > len || abs(v.z) > len)
		return 1;
		sqrdist = v.x * v.x + v.y * v.y + v.z * v.z;
		return sqrdist > len * len;
	}

	glm::dvec3 gravitacc(glm::dvec3 falling, glm::dvec3 dest, double mass)
	{
		glm::dvec3 ptr = dest - falling;
		return elenvec3(ptr, mass) ?
		glm::dvec3(0.0f) :
		ptr * (mass * GN / (getDist() * sqrdist));
	}

	namespace sp
	{
		static const float GN(6.67430151e-11f);
		// static const float C(299792458.0f);
		
		static float sqrdist;

		float getDist() {
			return sqrtf(sqrdist);
		}

		float cosvec3(glm::vec3 v, glm::vec3 av) {
			return glm::dot(v, av) / (glm::length(v) * glm::length(av));
		}

		glm::vec3 rotate(glm::vec3 /*normalized*/r_axis, glm::vec3 point, float angle)
		{
			glm::vec3 sub = point - r_axis * glm::length(point) * cosvec3(r_axis, point);
			return point
				+ glm::normalize(glm::cross(r_axis, sub)) * (sinf(angle) * glm::length(sub))
				- sub * (1.f - cosf(angle));
		}
		
		int elenvec3(glm::vec3 v, float len)
		{
			if (fabs(v.x) > len || fabs(v.y) > len || fabs(v.z) > len)
				return 1;
			sqrdist = v.x * v.x + v.y * v.y + v.z * v.z;
			return sqrdist > len * len;
		}

		glm::vec3 gravitacc(glm::vec3 falling, glm::vec3 dest, float mass)
		{
			glm::vec3 ptr = dest - falling;
			return elenvec3(ptr, mass) ?
				glm::vec3(0.0f) :
				ptr * (mass * GN / (getDist() * sqrdist));
		}
	}
}