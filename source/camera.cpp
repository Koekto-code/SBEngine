// 18 mar 2022

#include "camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include "physics.hpp"

Camera::Camera(glm::dvec3 position, glm::vec3 up, float yaw, float pitch, glm::vec3 forward)
{
	WorldPos = position;
	WorldUp = up;
	Forward = forward;
	Yaw = yaw;
	Pitch = pitch;
	updateVectors();
}

Camera::Camera(double posX, double posY, double posZ, float upX, float upY, float upZ, float yaw, float pitch, glm::vec3 forward) {
	WorldPos = glm::dvec3(posX, posY, posZ);
	WorldUp = glm::vec3(upX, upY, upZ);
	Forward = forward;
	Yaw = yaw;
	Pitch = pitch;
	updateVectors();
}

glm::dmat4 Camera::getViewMatrix()
{
	return glm::lookAt(WorldPos, WorldPos + (glm::dvec3)Forward, (glm::dvec3)Up);
}

void Camera::processKeyboard(int direction, double& dT)
{
	glm::vec3 mov = glm::vec3(0.f);
	
	if (direction & MOV_FW)
		mov = glm::normalize(glm::vec3(Forward.x, 0.f, Forward.z));
	else if (direction & MOV_BW)
		mov -= glm::normalize(glm::vec3(Forward.x, 0.f, Forward.z));
	if (direction & MOV_L)
		mov -= Right;
	else if (direction & MOV_R)
		mov += Right;
	if (direction & MOV_UP)
		mov += WorldUp;
	else if (direction & MOV_DN)
		mov -= WorldUp;
	
	if (direction) // mov != glm::vec3(0.f)
		camV += (glm::dvec3)glm::normalize(mov) * (dT * 40.0);
}

void Camera::processMouseMov(double& xpos, double& ypos)
{
	static float prev_x = xpos, prev_y = ypos;
	if (fInput & M_INIT)
	{
		prev_x = xpos;
		prev_y = ypos;
		fInput ^= M_INIT;
	}

	Yaw += ((float)xpos - prev_x) * MouseS;
	Pitch += (prev_y - (float)ypos) * MouseS;
	
	prev_x = xpos; prev_y = ypos;
	
	float angle = fabs(Pitch);
	if (angle > 1.57f)
		Pitch = Pitch > 0.f ? 1.57f : -1.57f;
	
	angle = fabs(Yaw);
	if (angle > TWO_PI)
		Yaw += Yaw > 0.f ? -TWO_PI : TWO_PI;

	updateVectors();
}

void Camera::scrollCallback(double& yoffset, int& mods)
{
	fInput |= VIEW_CHANGE;
	if (mods & GLFW_MOD_ALT)
	{
		FoV -= (float)yoffset * 0.03f;
		if (FoV < 0.07f)
			FoV = 0.07f;
		else if (FoV > 2.1f)
			FoV = 2.1f;
	}
	else
	{
		RenderDist += 25.f * (float)yoffset;
		if (RenderDist < 25.f)
			RenderDist = 25.f;
	}
}

void Camera::updatePhysics(double& dT)
{
	if (camV != glm::vec3(0.f))
	{
		glm::vec3 vel = camV;
		camV -= glm::normalize(camV) * (4.f * (float)dT);
		if (ph::sp::cosvec3(vel, camV) < 0.3f)
		camV = glm::vec3(0.f);
	}
	WorldPos += (glm::dvec3)camV * dT;
}

void Camera::updateVectors()
{
	glm::vec3 fw;
	fw.x = fw.z = cos(Pitch);
	fw.x *= cos(Yaw);
	fw.y = sin(Pitch);
	fw.z *= sin(Yaw);
	Forward = glm::normalize(fw);
	
	Right = glm::normalize(glm::cross(Forward, WorldUp));
	Up = glm::normalize(glm::cross(Right, Forward));
}