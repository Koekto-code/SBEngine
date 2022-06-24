// 24 june 2022
// SB Engine v0.1.0-alpha

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <common.hpp>
#include <glm/gtc/type_ptr.hpp>

#define CAM_QUART_PI 0.78539816F
#define CAM_HALF_PI 1.57079633F
#define CAM_PI 3.14159265F
#define CAM_TWO_PI 6.2831853F

// Defaults
#define CAM_YAW -CAM_QUART_PI
#define CAM_PITCH 0.0f
#define CAM_ACCELERATION 2.5F
#define CAM_MOUSE_SENS 0.00174533F
#define CAM_RENDER_DIST 500.0f

// Camera direction flags
#define CAM_MOV_FW	1 // W
#define CAM_MOV_BW	2 // S
#define CAM_MOV_L	4 // A
#define CAM_MOV_R	8 // D
#define CAM_MOV_UP	16 // Space
#define CAM_MOV_DN	32 // Shift

// Input flags
#define CAM_MOUSE_ENTRY	1 // Cursor have just been disabled
#define CAM_VIEW_CHANGE	2 // FOV or render distance changed

class Camera
{
public:
	glm::dvec3 wPos; // World position
	glm::vec3 wVel; // World velocity
	
	glm::vec3 forward;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 wUp;

	float yaw; 
	float pitch;

	float mouseSensitivity = CAM_MOUSE_SENS;
	float fov = CAM_QUART_PI;
	float renderDist = CAM_RENDER_DIST;
	
	// Flags storage
	int fInput = 0;

	Camera (
		const glm::dvec3& position = glm::dvec3(0.0), const glm::vec3& up = glm::vec3(0.f, 1.f, 0.f),
		const float yaw = CAM_YAW, const float pitch = CAM_PITCH, const glm::vec3& forward = glm::vec3(0.f, 0.f, -1.f)
	);

	void applyVelDelta(const int direction, const double dT); // direction contains flags from CAM_MOV
	void applyRotation(const float xpos, const float ypos); // Rotate camera by mouse
	void glfwScrollCallback(const float yoffset, const int mods);
	void updatePhysics(const double dT);

	glm::mat4 getViewMatrix();
private:
	void updateVectors();
};

#endif // CAMERA_HPP