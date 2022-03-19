// 18 mar 2022

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/gtc/type_ptr.hpp>

#define QUART_PI 0.78539816F
#define HALF_PI 1.57079633F
#define PI 3.14159265F
#define TWO_PI 6.2831853F

// Defaults
#define YAW -QUART_PI
#define PITCH 0.0F
#define ACCELERATION 2.5F
#define SENSITIVITY 0.00174533F
#define RENDER_DIST 500.0F

class Camera
{
public:
	glm::dvec3 WorldPos;
	glm::vec3 camV;
	
	glm::vec3 Forward;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	float Yaw; 
	float Pitch;

	float MouseS = SENSITIVITY;
	float FoV = QUART_PI;
	float RenderDist = RENDER_DIST;
	int fInput = 0;

	Camera(
		glm::dvec3 position = glm::dvec3(0.0), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = YAW, float pitch = PITCH, glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f)
	);

	Camera(
		double posX, double posY, double posZ, float upX, float upY, float upZ,
		float yaw, float pitch, glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f)
	);

	glm::dmat4 getViewMatrix();
	void processKeyboard(int direction, double& dT);
	void processMouseMov(double& xpos, double& ypos);
	void scrollCallback(double& yoffset, int& mods);
	void updatePhysics(double& dT);
	
	// Camera movement flags
	const int MOV_FW =	0x1; // W
	const int MOV_BW =	0x2; // S
	const int MOV_L =	0x4; // A
	const int MOV_R =	0x8; // D
	const int MOV_UP =	0x10; // Space
	const int MOV_DN =	0x20; // Shift
	
	// Input flags
	const int M_INIT = 0x1; // GLFW mode switched to GLFW_CURSOR_DISABLED
	const int VIEW_CHANGE = 0x2; // FOV or render distance changed

private:
	void updateVectors();
};

#endif // CAMERA_HPP