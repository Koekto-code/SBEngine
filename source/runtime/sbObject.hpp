// 24 june 2022
// SB Engine v0.1.0-alpha

#ifndef SB_OBJECT_HPP
#define SB_OBJECT_HPP

/* Strut-based objects class. */

#include <common.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <vector>
#include <string>

#include <runtime/model.hpp>
#include <runtime/shader.hpp>

#define SB_OBJECT_INIT_HARDNESS 1.2e7

typedef struct {
	glm::dvec3 pos;
	glm::dvec3 vel;
	glm::dvec3 force;
} massPoint;

typedef struct {
	glm::uvec2 index;
	double len;
} sbStrut;

class sbObject
{
public:
	sbObject();
	sbObject(const std::string& path, const double mass);

	void loadModel(const std::string& path, const double hardness = SB_OBJECT_INIT_HARDNESS, const double viscosity = 0.5);
	void setMass(const double mass);

	void translate(const glm::dvec3& delta);
	void setPosition(const glm::dvec3& pos);
	void accelerate(const glm::dvec3& delta);
	void setVelocity(const glm::dvec3& vel, const bool sync = true);

	glm::dvec3 getPosition();
	glm::dvec3 getVelocity();
	
	size_t getNumMassPoints();
	massPoint* getMassPoints();

	void updatePhysics(const double dT);
	void glRender(Shader& shader);
	void updateMeshVBOs(const glm::dvec3& center);

	double hard_k, visc_k;

	// Protected variables & functions summary:
	// Is modified: CLF- when importing model from file, CLM- in sbObject::loadModel(), CD- dynamically
	// Is used (not by itself): ULF, ULM, UD likewise.
protected:
	Model model; // GL model. CLF | ULM

	double mass; // CLM | UD
	double pointMass; // CLM | UD

	// Convert linear index to (#mesh, #vertex). ULM | UD
	glm::uvec2 getMeshVertIndex(unsigned long n);

	// Convert model indices (which may repeat) to massPoints indices. CLM | ULM | UD
	std::vector<unsigned long> mpIndices; 
	
	// A set of struts to keep model shape. CLM | UD
	std::vector<sbStrut> mpStruts;
	std::vector<massPoint> massPoints; // CLM | ULM | CD | UD
};

#endif