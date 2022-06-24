// 24 june 2022
// SBView v0.1.0-alpha

#include <common.hpp>

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <runtime/glUtil.h>
#include <runtime/localMath.hpp>
#include <runtime/camera.hpp>
#include <runtime/sbObject.hpp>
#include <runtime/print.hpp>
#include <runtime/world.hpp>

#include <app/argparser.hpp>
#include <app/graphics.hpp>
#include <app/callbacks.hpp>

#include <cmath>
#include <vector>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <fstream>

static GLFWwindow* window;

static double currentTime = 0.0;
static double savedTime = 0.0;
static double deltaTime = 0.0;

static sbObject sb_object;
static Camera camera(glm::dvec3(135.0, 1.0, 2.0));

Camera* getCurrentCamera() { return &camera; }
sbObject* getObject() { return &sb_object; }

static const char* applyOptions(int argc, char** argv); // Return model path parsed from options
static void updateTimeDelta();
static void processInput(GLFWwindow* wnd, double dT);

int main(int argc, char** argv)
{
	std::ofstream logfile("latest.log", std::fstream::trunc);
	if (logfile.is_open())
		logfile.close();
	const char* loadpath = applyOptions(argc, argv);

	int err = graphics::init(window);
	if (err) {
		logfile.open("latest.log", std::fstream::app);
		if (logfile.is_open())
		{
			logfile << "Failed to init graphics: error " << err << "\n";
			logfile.close();
		}
		return err;
	}
	
	sb_object.loadModel(loadpath, g_hardness);
	sb_object.setPosition(glm::dvec3(140.0, 0.0, 0.0));

	glfwSetKeyCallback(window, input::keyCallbackEvent);
	glfwSetFramebufferSizeCallback(window, input::framebufResizeCallbackEvent);
	glfwSetMouseButtonCallback(window, input::mouseButtonCallbackEvent);
	glfwSetCursorPosCallback(window, input::cursorPosCallbackEvent);
	glfwSetScrollCallback(window, input::scrollCallbackEvent);
	glfwSwapInterval(1);
	
	camera.fInput |= CAM_VIEW_CHANGE;
	/* Rendering loop */
	while (!glfwWindowShouldClose(window)) {
		updateTimeDelta();
		processInput(window, deltaTime);
		graphics::display(camera, sb_object);
		glfwSwapBuffers(window);
	}

	graphics::shutdown();
	return 0;
}

static const char* applyOptions(int argc, char** argv)
{
	if (argp::exist("--help", argc, argv))
	{
		std::cout <<
		"Usage: " << argv[0] << " [options]...\n"
		"Options:\n"
		"  -m <file>  \t\tLoad the model from <file>.\n"
		"  -h <float> \t\tSet hardness to <float>. Value is non-negative.\n"
		"  -v <float> \t\tSet viscosity to <float>. Values are from 0 to 1.\n"
		"  -t <float> \t\tSet time multiplier to <float>.\n"
		"  -p <int>   \t\tMultiply physics frequency by <unsigned>. Value is positive.\n"
		"  --m1 <int>   \t\tSet first marker to <int>-th vertex of model. Value is non-negative.\n"
		"  --m2 <int>   \t\tSet second marker to <int>-th vertex of model. Value is non-negative.\n"
		"  --mass <float>\tSpecify initial planet mass.\n"
		"\n";
		exit(0);
	}
	
	char* argstr;
	
	if ((argstr = argp::getopt("-p", argc, argv))) {
		int prec = std::atoi(argstr);
		if (prec > 0)
			g_tPrec = prec;
	}
	if ((argstr = argp::getopt("-t", argc, argv))) {
		float mul = std::atof(argstr);
		if (! (std::isnan(mul) || std::isinf(mul)))
			g_tMul = mul;
	}
	if ((argstr = argp::getopt("-h", argc, argv))) {
		float hh = std::atof(argstr);
		if (hh >= 0.f && !(std::isnan(hh) || std::isinf(hh)))
			g_hardness = hh;
	}
	if ((argstr = argp::getopt("-v", argc, argv))) {
		float visc = std::atof(argstr);
		if (visc >= 0.f && visc < 1.f)
			g_visc = visc;
	}
	if ((argstr = argp::getopt("--m1", argc, argv, true))) {
		int mo = std::atoi(argstr);
		if (mo >= 0)
			g_markerIndex1 = mo;
	}
	if ((argstr = argp::getopt("--m2", argc, argv, true))) {
		int mv = std::atoi(argstr);
		if (mv >= 0)
			g_markerIndex2 = mv;
	}
	if ((argstr = argp::getopt("--mass", argc, argv, true))) {
		float mass = std::atof(argstr);
		if (! (std::isnan(mass) || std::isinf(mass)))
			g_plMass = mass;
	}
	if ((argstr = argp::getopt("-m", argc, argv)))
		return argstr;
	return "./resources/models/bflyscrew.obj";
}

static void updateTimeDelta()
{
	static bool phready = false;
	currentTime = glfwGetTime();
	deltaTime = (currentTime - savedTime);
	if (!phready) {
		deltaTime = 0.0;
		phready = currentTime > 2.0;
	}
	savedTime = currentTime;
}

static void processInput(GLFWwindow* wnd, double dT)
{
	glfwPollEvents();
	int mouseState = input::getVal(input::MOUSE_STATE);
	int keyMods = input::getVal(input::KEY_MODS);

	if (glfwGetKey(wnd, GLFW_KEY_K))
	{
		if (keyMods & GLFW_MOD_CONTROL)
			g_tMul = 1.0;
		else if (keyMods & GLFW_MOD_ALT)
			g_tMul *= 0.97;
		else g_tMul *= 1.034;
	}
	
	graphics::g_markers[0].orig = graphics::g_markers[1].orig = sb_object.getPosition();
	graphics::g_markers[0].v = sb_object.getMassPoints()[g_markerIndex1].pos;
	graphics::g_markers[1].v = sb_object.getMassPoints()[g_markerIndex2].pos;

	if (glfwGetKey(wnd, GLFW_KEY_R)) {
		sb_object.setPosition(glm::dvec3(140.0, 0.0, 0.0));
		sb_object.setVelocity(glm::dvec3(0.0, 0.0, 0.0));
	}

	// Camera movement
	{
		int cmov = 0;

		if (glfwGetKey(wnd, GLFW_KEY_W))
			cmov = CAM_MOV_FW;
		else if (glfwGetKey(wnd, GLFW_KEY_S))
			cmov = CAM_MOV_BW;
		if (glfwGetKey(wnd, GLFW_KEY_A))
			cmov |= CAM_MOV_L;
		else if (glfwGetKey(wnd, GLFW_KEY_D))
			cmov |= CAM_MOV_R;
		if (glfwGetKey(wnd, GLFW_KEY_SPACE))
			cmov |= CAM_MOV_UP;
		else if (input::getVal(input::KEY_MODS) & GLFW_MOD_SHIFT)
			cmov |= CAM_MOV_DN;
		
		camera.applyVelDelta(cmov, dT);
		camera.updatePhysics(dT);
	}
	
	static bool objectGrabbed = false;
	if (mouseState & (1 << GLFW_MOUSE_BUTTON_RIGHT))
	{
		/* Grab or push the object */
		glm::dvec3 grab = camera.wPos + glm::dvec3(camera.forward * 8.f);
		glm::dvec3 objCoords = sb_object.getPosition();
		glm::dvec3 rope = grab - objCoords;

		if (!lcm::elenvec3(rope, 8.1))
		{
			if (mouseState & (1 << GLFW_MOUSE_BUTTON_LEFT))
			{
				// sb_object.setPosition(grab);
				// sb_object.setVelocity(camera.wVel);
				sb_object.setVelocity(camera.wVel, keyMods & GLFW_MOD_ALT);
			}
			else if (keyMods & GLFW_MOD_ALT)
			{
				sb_object.accelerate((glm::dvec3)camera.forward * (dT * 500.0));
				objectGrabbed = false;
			}
			else
			{
				if (!objectGrabbed || mouseState & (1 << GLFW_MOUSE_BUTTON_LEFT))
					sb_object.setVelocity(camera.wVel);
				
				objectGrabbed = true;
				
				// Indicates approx/moving away speed (if negative then approximating)
				double vcos = lcm::cosvec3(rope, -sb_object.getVelocity());
				glm::dvec3 acc = rope * (/* glm::length(rope) */ 500.0);

				// Let the amplitude fade
				if ((vcos < 0.0))
					acc *= 0.5;
				else acc *= 1.5;
				
				sb_object.accelerate(acc * dT);
				// sb_object.setPosition(grab);
				// sb_object.setVelocity(camera.wVel, false);
			}
		}
	}
	else
	{
		objectGrabbed = false;
		if (mouseState & (1 << GLFW_MOUSE_BUTTON_LEFT))
			sb_object.setVelocity(glm::dvec3(0.0), keyMods & GLFW_MOD_ALT);
	}

	sb_object.hard_k = g_hardness;
	sb_object.visc_k = g_visc;

	double objdt = g_tMul * dT;

	// Rotate object
	if (glfwGetKey(wnd, GLFW_KEY_T))
	{
		massPoint* mpts = sb_object.getMassPoints();
		glm::dvec3 center = sb_object.getPosition();
		glm::dvec3 f = glm::normalize(glm::cross(
			mpts[g_markerIndex1].pos - center,
			mpts[g_markerIndex2].pos - center
		)) * g_forceMul;

		double test = glm::length(f);
		if (std::abs(test - g_forceMul) < 0.01)
		{
			mpts[g_markerIndex1].vel += f * objdt;
			mpts[g_markerIndex2].vel -= f * objdt;
		}
	}

	objdt /= g_tPrec;
	for (long loop = 0; loop < g_tPrec; ++loop) {
		sb_object.updatePhysics(objdt);
	}
}
