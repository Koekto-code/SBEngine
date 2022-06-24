// 24 june 2022
// SB Engine v0.1.0-alpha

#include <runtime/sbObject.hpp>
#include <runtime/localMath.hpp>

#include <GLFW/glfw3.h>

#include <cassert>
#include <cmath>

// main.cpp variables
extern const double g_plRadius;
extern double g_plMass;
extern double g_tMul;

// uvec2(5, 2) == uvec2(2, 5) if they are struts
static bool uv2ceq(glm::uvec2 v0, glm::uvec2 v1)
{
	if (v0 == v1)
		return true;
	
	// Swap without temporary
	v1.x ^= v1.y;
	v1.y ^= v1.x;
	v1.x ^= v1.y;
	return (v0 == v1);
}

sbObject::sbObject()
{
}

sbObject::sbObject(const std::string& path, const double mass)
{
	loadModel(path);
	setMass(mass);
}

void sbObject::loadModel(const std::string& path, const double hardness, const double viscosity)
{
	model.Import(path);

	// Getting unique coords and binding them to GL model
	{
		unsigned long maxLen = 0;
		for (unsigned long i = 0; i < model.meshes.size(); ++i)
			maxLen += model.meshes[i].vertices.size();
		
		mpIndices.resize(maxLen);
		massPoints.resize(maxLen);
		
		unsigned long mpAllocated = 0;
		for (unsigned long i = 0; i < maxLen; ++i)
		{
			glm::uvec2 mvi_i = getMeshVertIndex(i);

			// Find if current vertex coords are already stored
			for (unsigned long k = 0; k < massPoints.size(); ++k)
			{
				if (glm::dvec3(model.meshes[mvi_i.x].vertices[mvi_i.y].Position) == massPoints[k].pos)
				{
					mpIndices[i] = k;
					goto next;
				}
			}
			// Insert new element
			mpIndices[i] = mpAllocated;
			massPoints[mpAllocated++].pos = glm::dvec3(model.meshes[mvi_i.x].vertices[mvi_i.y].Position);
			next: continue;
		}
		massPoints.resize(mpAllocated);
	}

	// Connect weights with struts
	{
		unsigned long struts = 0;
		
		// Define max possible struts by number of vertex connections
		for (unsigned long i = 0; i < model.meshes.size(); ++i)
		{
			struts += model.meshes[i].triangles.size();
			struts += (model.meshes[i].edges.size() >> 1);
		}

		mpStruts.resize(struts);

		// Parse struts
		for (unsigned long i = 0, currentVerts = 0, filled = 0; i < model.meshes.size(); ++i)
		{
			Mesh& this_mesh = model.meshes[i];
			unsigned long this_triangles = this_mesh.triangles.size();
#if 		_DEBUG
			assert(! (this_triangles % 3));
#endif
			for (unsigned long k = 0; k < this_triangles; k += 3)
			{
				for (unsigned e = 0; e < 3; )
				{
					mpStruts[filled].index.x = mpIndices[currentVerts + this_mesh.triangles[k + e]]; ++e;
					mpStruts[filled].index.y = mpIndices[currentVerts + this_mesh.triangles[k + (e % 3)]];
					
					mpStruts[filled].len = glm::distance(
						massPoints[mpStruts[filled].index.x].pos,
						massPoints[mpStruts[filled].index.y].pos
					);

					++filled;
				}
			}
			
			unsigned long this_edges = this_mesh.edges.size();
#if 		_DEBUG
			assert(! (this_edges & 1));
#endif
			for (unsigned long e = 0; e < this_edges; )
			{
				mpStruts[filled].index.x = mpIndices[currentVerts + this_mesh.edges[e]]; ++e;
				mpStruts[filled].index.y = mpIndices[currentVerts + this_mesh.edges[e]]; ++e;

				mpStruts[filled].len = glm::distance(
					massPoints[mpStruts[filled].index.x].pos,
					massPoints[mpStruts[filled].index.y].pos
				);
				++filled;
			}
			
			currentVerts += this_mesh.vertices.size();
		}

		// Delete repeating struts
		for (unsigned long i = 0; i < mpStruts.size(); ++i)
		{
			for (unsigned long k = 0; k < i; ++k)
			{
				if (uv2ceq(mpStruts[k].index, mpStruts[i].index))
				{
					auto it = mpStruts.begin();
					std::advance(it, i);
					mpStruts.erase(it);
					--struts;
					--i; // Go to previous (i.e. current after erasing) value to avoid skipping
				}
			}
		}

#if		_DEBUG
		assert(! (std::isnan(hardness) || std::isinf(hardness)));
		assert(viscosity > 0.0 && viscosity < 1.0);
#endif
		hard_k = hardness;
		visc_k = viscosity;
	}
}

void sbObject::setMass(const double mass)
{
#if _DEBUG
	assert(massPoints.size());
#endif
	this->mass = mass;
	pointMass = mass / massPoints.size();
}

void sbObject::translate(const glm::dvec3& delta)
{
	for (unsigned long i = 0; i < massPoints.size(); ++i)
		massPoints[i].pos += delta;
}

void sbObject::setPosition(const glm::dvec3& pos)
{
	translate(pos - getPosition());
}

void sbObject::accelerate(const glm::dvec3& delta)
{
	for (unsigned long i = 0; i < massPoints.size(); ++i)
		massPoints[i].vel += delta;
}

void sbObject::setVelocity(const glm::dvec3& vel, const bool sync)
{
	if (sync)
	{
		// Disable rotation
		for (unsigned long i = 0; i < massPoints.size(); ++i)
			massPoints[i].vel = vel;
		return;
	}
	accelerate(vel - getVelocity());
}

glm::dvec3 sbObject::getPosition()
{
	glm::dvec3 avm(0.0);
	for (unsigned long i = 0; i < massPoints.size(); ++i)
		avm += massPoints[i].pos;
	
	avm /= massPoints.size();
	return avm;
}

glm::dvec3 sbObject::getVelocity()
{
	glm::dvec3 avm(0.0);
	for (unsigned long i = 0; i < massPoints.size(); ++i)
		avm += massPoints[i].vel;
	
	avm /= massPoints.size();
	return avm;
}

size_t sbObject::getNumMassPoints() { return massPoints.size(); }

massPoint* sbObject::getMassPoints() { return &massPoints[0]; }

void sbObject::updatePhysics(const double dT)
{
	// Apply gravitation
	accelerate(lcm::gravitacc(getPosition(), glm::dvec3(0.0), g_plMass) * dT);
	
	// Make an object keep its shape
	for (unsigned long i = 0; i < mpStruts.size(); ++i)
	{
		unsigned long p0 = mpStruts[i].index.x, p1 = mpStruts[i].index.y;
		glm::dvec3 diff = massPoints[p0].pos - massPoints[p1].pos;
		
		double offset = 1.0 - (glm::dot(diff, diff) / (mpStruts[i].len * mpStruts[i].len));
		double dotv = glm::dot(diff, massPoints[p0].vel - massPoints[p1].vel);
		
		diff = (offset * hard_k) * glm::normalize(diff);
		
		if ((dotv < 0.0) xor (offset < 0.0))
			diff *= (1.0 + visc_k);
		
		massPoints[p0].force += diff;
		massPoints[p1].force -= diff;
	}

	// Process collisions
	for (unsigned long i = 0; i < massPoints.size(); ++i)
	{
		double offset = g_plRadius * g_plRadius - glm::dot(massPoints[i].pos, massPoints[i].pos);
		if (offset > 0.0)
		{
			double forceMod = (hard_k * offset);
			glm::dvec3 nextPosition = massPoints[i].pos + (massPoints[i].vel * dT);

			if (glm::dot(nextPosition, nextPosition) < g_plRadius * g_plRadius)
				forceMod *= (1.0 + visc_k);

			massPoints[i].force += forceMod * glm::normalize(massPoints[i].pos);
		}
	}

	// Update values
	for (unsigned long i = 0; i < massPoints.size(); ++i)
	{
		massPoints[i].vel += massPoints[i].force * dT;
		massPoints[i].pos += massPoints[i].vel * dT;
		massPoints[i].force = glm::dvec3(0.0);
	}
}

void sbObject::glRender(Shader& shader)
{
	glm::dvec3 coords = getPosition();
	glm::mat4 transform = glm::translate(glm::mat4(1.f), (glm::vec3)coords);

	updateMeshVBOs(coords);

	shader.setMat4("transform", transform);
	model.Render(shader);
}

void sbObject::updateMeshVBOs(const glm::dvec3& center)
{
	for (unsigned long i = 0; i < mpIndices.size(); ++i)
	{
		glm::uvec2 index = getMeshVertIndex(i);
		model.meshes[index.x].vertices[index.y].Position = massPoints[mpIndices[i]].pos - center;
	}
	for (unsigned long i = 0; i < model.meshes.size(); ++i)
	{
		Mesh &refm = model.meshes[i];
		glBindVertexArray(refm.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, refm.VBO);
		glBufferData(GL_ARRAY_BUFFER, refm.vertices.size() * sizeof(Vertex), &refm.vertices[0], GL_DYNAMIC_DRAW);
	}
}

glm::uvec2 sbObject::getMeshVertIndex(unsigned long n)
{
	for (unsigned long i = 0; i < model.meshes.size(); ++i)
	{
		if (n < model.meshes[i].vertices.size())
			return glm::uvec2(i, n);
		n -= model.meshes[i].vertices.size();
	}
	throw std::length_error("reached end of mesh vertices");
}
