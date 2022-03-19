// 15 mar 2022

#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <cmath>
#include <vector>
#include <cassert>
#include "physics.hpp"
#include "misc.hpp"

class Object // needs revision!
{
public:
	glm::dvec3 coords = glm::dvec3(0.0); // world position
	glm::dvec3 speed = glm::dvec3(0.0);

	// axis of rotation (not unit vector). Its length equals rotating speed
	glm::dvec3 rotateVec = glm::dvec3(0.0);
	double rotateRads = 0.0; // angular position

	float* surface; // visible model (rotating)
	uint32_t surface_len; // sizeof(array_that_is_used_by_surface) / sizeof(float)

	glm::vec3* masspts = nullptr; // mass model (rotating)
	uint16_t mpts = 0; // quantity of mass points

	void set_mass(float m, glm::vec3* ptr = nullptr, uint16_t q = 0);
	void set_surface(float* source, size_t len, uint8_t stride = 5);
	void rotate(const double& dT);
	void applyForce(glm::dvec3 force, glm::dvec3 location, const double& dT);
	void updatePhysics(const double& dT, bool keypressed = false);
	void passv(unsigned f, unsigned s, unsigned t, glm::dvec3 vr);
	
// private:
	double r; // maximum of vertice distance from center of gravity
	float mass = 0.f;

	float m_unit = 0.f; // mass of each point
    glm::vec3* init_masspts = nullptr; // mass points' coords (static)
    float* init_surface; // model (static)
	
    uint8_t stride; // All models in this version are 5-based
	glm::vec3 rvn; // normalized rotating vector
};

#endif