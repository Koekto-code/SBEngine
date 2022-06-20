// 20 june 2022
// SB Engine v0.4.0-alpha

#include <runtime/localMath.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

NAMESPACE_BEGIN(lcm)

const double GN(6.674301515e-11);
// const double C(299792458.0);

static double sqrdist;

double getDist() {
	return sqrt(sqrdist);
}

double cosvec3(glm::dvec3 const& v, glm::dvec3 const& av) {
	return glm::dot(v, av) / (sqrt(glm::dot(v, v) * glm::dot(av, av)));
}

glm::dvec3 rotate(glm::dvec3 const& point, const double angle, glm::dvec3 /*normalized*/ const& r_axis)
{
	const glm::dvec3 sub = point - r_axis * glm::length(point) * cosvec3(r_axis, point);
	return point
	+ glm::normalize(glm::cross(r_axis, sub)) * (sin(angle) * glm::length(sub))
	- sub * (1.0 - cos(angle));
}

bool elenvec3(glm::dvec3 const& v, const double len)
{
	if (abs(v.x) > len || abs(v.y) > len || abs(v.z) > len)
	return 1;
	sqrdist = v.x * v.x + v.y * v.y + v.z * v.z;
	return sqrdist > len * len;
}

// Working with problems, isn't used
/*
glm::dmat4 decomposeRotation
(
	const glm::dvec3& k, const glm::dvec3& v, // Before rotating
	const glm::dvec3& ck, const glm::dvec3& cv // After
)
{
	double angle0, angle1;
	glm::dvec3 axis0;
	glm::dvec3 cvi, cvl;
	
	angle0 = cosvec3(k, ck);
	if (abs(angle0) > (1.0 - 1.e-9))
		cvi = v * angle0;
	else
	{
		angle0 = acos(angle0);
		axis0 = glm::normalize(glm::cross(k, ck));
		cvi = rotate(v, angle0, axis0);
	}
	
	glm::dvec3 axis1 = glm::normalize(ck);
	
	// Adjust points to get correct angle
	cvi = cvi - axis1 * (glm::dot(ck, cvi) / glm::length(cvi));
	cvl = cv - axis1 * (glm::dot(ck, cv) / glm::length(cv));
	
	angle1 = acos(cosvec3(cvi, cvl));
	
	// Combine transforms
	glm::dmat4 tf(1.0);
	tf = glm::rotate(tf, angle1, axis1);
	tf = glm::rotate(tf, angle0, axis0);
	
	return tf;
}
*/

glm::dvec3 gravitacc(glm::dvec3 const& falling, glm::dvec3 const& dest, double mass)
{
	const glm::dvec3 ptr = dest - falling;
	return elenvec3(ptr, mass) ? glm::dvec3(0.0) :
		ptr * (mass * GN / (getDist() * sqrdist));
}

NAMESPACE_BEGIN(sp) // single precision

const float GN(6.67430151e-11f);
// const float C(299792458.0f);

static float sqrdist;

float getDist() {
	return sqrtf(sqrdist);
}

float cosvec3(glm::vec3 const& v, glm::vec3 const& av) {
	return glm::dot(v, av) / (sqrtf(glm::dot(v, v) * glm::dot(av, av)));
}

glm::vec3 rotate(glm::vec3 const& point, const float angle, glm::vec3 /*normalized*/ const& r_axis)
{
	const glm::vec3 sub = point - r_axis * glm::length(point) * cosvec3(r_axis, point);
	return point
		+ glm::normalize(glm::cross(r_axis, sub)) * (sinf(angle) * glm::length(sub))
		- sub * (1.f - cosf(angle));
}

bool elenvec3(glm::vec3 const& v, const float len)
{
	if (fabs(v.x) > len || fabs(v.y) > len || fabs(v.z) > len)
		return 1;
	sqrdist = v.x * v.x + v.y * v.y + v.z * v.z;
	return sqrdist > len * len;
}

// Working with problems, isn't used
/*
glm::mat4 decomposeRotation
(
	const glm::vec3& k, const glm::vec3& v, // Before rotating
	const glm::vec3& ck, const glm::vec3& cv // After
)
{
	float angle0, angle1;
	glm::vec3 axis0;
	glm::vec3 cvi, cvl;
	
	angle0 = cosvec3(k, ck);
	if (fabs(angle0) > (0.9998f))
		cvi = v * angle0;
	else
	{
		angle0 = acosf(angle0);
		axis0 = glm::normalize(glm::cross(k, ck));
		cvi = rotate(v, angle0, axis0);
	}
	
	glm::vec3 axis1 = glm::normalize(ck);
	
	// Adjusting points to get correct angle
	cvi = cvi - axis1 * (glm::dot(ck, cvi) / glm::length(cvi));
	cvl = cv - axis1 * (glm::dot(ck, cv) / glm::length(cv));
	
	angle1 = acosf(cosvec3(cvi, cvl));
	
	// Combine transforms
	glm::mat4 tf(1.f);
	tf = glm::rotate(tf, angle1, axis1);
	tf = glm::rotate(tf, angle0, axis0);
	
	return tf;
}
*/

glm::vec3 gravitacc(glm::vec3 const& falling, glm::vec3 const& dest, const float mass)
{
	glm::vec3 ptr = dest - falling;
	return elenvec3(ptr, mass) ?
		glm::vec3(0.0f) :
		ptr * (mass * GN / (getDist() * sqrdist));
}

NAMESPACE_END(sp)
NAMESPACE_END(lcm)