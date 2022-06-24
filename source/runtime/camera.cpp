// 24 june 2022
// SB Engine v0.1.0-alpha

#include <runtime/camera.hpp>
#include <runtime/localMath.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

Camera::Camera (
	const glm::dvec3& position, const glm::vec3& up,
	const float yaw, const float pitch, const glm::vec3& forward
)
{
	wPos = position;
	wUp = up;
	this->forward = forward;
	this->yaw = yaw;
	this->pitch = pitch;

	updateVectors();
	// fInput |= CAM_MOUSE_ENTRY;
}

glm::mat4 Camera::getViewMatrix()
{
	glm::vec3 spwPos = glm::vec3(wPos);
	return glm::lookAt(spwPos, spwPos + forward, up);
}


void Camera::applyVelDelta(const int direction, const double dT)
{
	glm::vec3 mov = glm::vec3(0.f);
	
	if (direction & (CAM_MOV_FW | CAM_MOV_BW))
	{
		mov = glm::normalize(glm::vec3(forward.x, 0.f, forward.z));
		if (direction & CAM_MOV_BW) mov = -mov;
	}
	if (direction & CAM_MOV_L)
		mov -= right;
	else if (direction & CAM_MOV_R)
		mov += right;
	if (direction & CAM_MOV_UP)
		mov += wUp;
	else if (direction & CAM_MOV_DN)
		mov -= wUp;
	
	if (mov != glm::vec3(0.f))
		wVel += glm::normalize(mov) * ((float)dT * 40.f);
}

void Camera::applyRotation(const float xpos, const float ypos)
{
	static float prev_x = xpos, prev_y = ypos;
	if (fInput & CAM_MOUSE_ENTRY)
	{
		prev_x = xpos;
		prev_y = ypos;
		fInput ^= CAM_MOUSE_ENTRY;
	}

	yaw += (xpos - prev_x) * mouseSensitivity;
	pitch += (prev_y - ypos) * mouseSensitivity;
	
	prev_x = xpos; prev_y = ypos;
	
	if (fabs(pitch) > 1.57f) {
		pitch = pitch < 0.f ? -1.57f : 1.57f;
	}
	
	if (fabs(yaw) > CAM_TWO_PI) {
		yaw += yaw < 0.f ? CAM_TWO_PI : -CAM_TWO_PI;
	}

	updateVectors();
}

void Camera::glfwScrollCallback(const float yoffset, const int keyMods)
{
	if (keyMods & GLFW_MOD_ALT)
	{
		fov -= yoffset * 0.03f;
		if (fov < 0.07f)
			fov = 0.07f;
		else if (fov > 2.1f)
			fov = 2.1f;
	}
	else {
		renderDist += 25.f * yoffset;
		if (renderDist < 25.f)
			renderDist = 25.f;
	}
	fInput |= CAM_VIEW_CHANGE;
}

void Camera::updatePhysics(const double dT)
{
	if (wVel != glm::vec3(0.f))
	{
		glm::vec3 vel = wVel;
		wVel -= glm::normalize(wVel) * (4.f * (float)dT);
		if (lcm::sp::cosvec3(vel, wVel) < 0.3f)
			wVel = glm::vec3(0.f);
	}
	wPos += (glm::dvec3)wVel * dT;
}

void Camera::updateVectors()
{
	glm::vec3 fw;
	fw.x = fw.z = cos(pitch);
	fw.x *= cos(yaw);
	fw.z *= sin(yaw);
	fw.y = sin(pitch);
	forward = glm::normalize(fw);
	
	right = glm::normalize(glm::cross(forward, wUp));
	up = glm::normalize(glm::cross(right, forward));
}