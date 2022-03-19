// 19 mar 2022
// ctest 0.3.0 alpha

#include <cassert>

// #include <iostream>
#include <cstdint>
#include <fstream>

#define GLFW_DLL
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

#include "physics.hpp"
#include "misc.hpp"

#include "shader.hpp"
#include "cube.h"
#include "sph.h"
#include "sphs.h"

#include "camera.hpp"

#define SKYBOX_INIT 3 // only 2 or 3

// graphics::flags
#define GR_VIEW_CHANGE 0x1

static std::ofstream logfile;
// static std::ofstream csvtable;

Camera camera(glm::dvec3(0.0, 0.0, 140.0));

static void processInput(GLFWwindow*);
static void keyboard102(GLFWwindow*, int, int, int, int);
static void framebuffer_size_callback(GLFWwindow*, int, int);
static void mouse_button_callback(GLFWwindow*, int, int, int);
static void cursor_pos_callback(GLFWwindow*, double, double);
static void mouse_scroll_callback(GLFWwindow*, double, double);

double g_sunMass = 25.e14; // kg
const double g_sunRadius = 125.0; // m

namespace graphics
{
	// OpenGL objects
	static unsigned int VBO, EBO, VAO,
		cubeVBO, cubeEBO, cubeVAO,
		sphVBO, sphEBO, sphVAO,
		skyboxVBO, skyboxEBO, skyboxVAO;
	static unsigned int texture0, texture1, texture2, texture3, cubeMap;

	static int flags = 0;
	static char skybox; // 3: display; 2: available, don't display; 0: not available
	static unsigned WinW, WinH;

	static void initPlanet();
	static void initSph();
	static void initCube();
	static void initSkybox();
	static int Init(); // returns 0 for success
	
	static void loadTexture(const char*, unsigned int*);
	static void loadCubemap(char*);
	
	static void display();
}

namespace app
{
	static char sphState = 2;
	static bool enableStruts = true;
	static void reset_spheres();
	static void rotative_figure_form();
	static void cube_form();
	static void resetScene();
	static void putStruts(double& time);
}

namespace input
{ // GLFW input
	static int kMods = 0; // shift control alt etc
	static int mMods = 0; // LMB RMB
	static float key_delay = 0.f; // keyboard delay
}
static GLFWwindow* window;

/* Timings */
static double currentTime = 0.0; // returned by GLFW
static double lastTime = 0.0;
static double deltaTime = 0.0;

static int tdiv = 77; // time divisor for spheres & other objects

struct Sphere {
	float r = 0.f;
	float mass = 5.f;
	glm::dvec3 coords; 
	glm::dvec3 speed = glm::dvec3(0.0);
	bool picked = false;
};

static Shader sunShader; // for the main sphere
static Shader sphShader; // for small spheres
static Shader cubeShader;
static Shader skyboxShader;

static float kf = 3.5e7f; // material hardness
static float visc = 0.7f; // spheres' viscosity (up to 1)

static std::vector<Sphere> sph;

#include "object.hpp"

Object cube;

glm::dvec3 sphCoords(unsigned num) {
	return sph[num].coords;
}

int main(int argc, char** argv)
{
	// forcetest.resize(500);
	if (graphics::Init() == -1)
		return -1;
	
	camera.fInput |= camera.VIEW_CHANGE;

	cube.set_mass(5.f /*, std::begin(massPts), sizeof(massPts) / sizeof(glm::vec3) */);
	cube.set_surface(std::begin(cubeVerts), sizeof(cubeVerts) / sizeof(float));
	cube.coords = glm::dvec3(140.0, 0.0, 0.0);

	app::cube_form();

	glBindVertexArray(0);

	sunShader.structShaders("../vs/planet.glsl", "../fs/planet.glsl");
	sphShader.structShaders("../vs/sphere.glsl", "../fs/sphere.glsl");
	cubeShader.structShaders("../vs/default.glsl", "../fs/default.glsl");
	
	if (graphics::skybox) {
		skyboxShader.structShaders("../vs/skybox.glsl", "../fs/skybox.glsl");
		skyboxShader.use();
		skyboxShader.setInt("cubeMap", 4);
	}
	
	sunShader.use();
	sunShader.setInt("texture3", 3);

	sphShader.use();
	sphShader.setInt("texture0", 0);
	sphShader.setInt("texture1", 1);

	cubeShader.use();
	cubeShader.setInt("texture2", 2);

	glClearColor(0.678f, 0.784f, 0.898f, 1.f); // sky color

	/* Rendering loop */
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		graphics::display();
	}

	glDeleteVertexArrays(1, &graphics::VAO);
	glDeleteBuffers(1, &graphics::VBO);
	glDeleteBuffers(1, &graphics::EBO);
	glDeleteVertexArrays(1, &graphics::cubeVAO);
	glDeleteBuffers(1, &graphics::cubeVBO);
	glDeleteBuffers(1, &graphics::cubeEBO);
	glDeleteVertexArrays(1, &graphics::sphVAO);
	glDeleteBuffers(1, &graphics::sphVBO);
	glDeleteBuffers(1, &graphics::sphEBO);
	if (graphics::skybox)
	{
		glDeleteVertexArrays(1, &graphics::skyboxVAO);
		glDeleteBuffers(1, &graphics::skyboxVBO);
		// graphics::skybox = 0;
	}
	glfwTerminate();
	return 0;
}

// Absolutely hard strut (inapplicable to many objects)
/* void hstrut(uint16_t s, uint16_t p, float dist, double& delta)
{
	glm::dvec3 rcoords = sph[p].coords - sph[s].coords;
	glm::dvec3 tc = sph[s].coords;

	sph[p].coords += sph[p].speed * delta;
	sph[s].coords += sph[s].coords * delta;
	glm::dvec3 ptr = sph[p].coords - sph[s].coords;
	glm::dvec3 sm = ptr - (double)dist * glm::normalize(ptr);
	
	sm *= (0.5 / delta);
	sph[s].coords = tc;
	sph[p].coords = tc + (glm::dvec3)rcoords;
	sph[s].speed += sm;
	sph[p].speed -= sm;
} */

// No kinetic energy loss
/* void strut(uint16_t s, uint16_t p, double slen, double& dT)
{
	glm::dvec3 vf = sph[p].coords - sph[s].coords;
	double offset = slen - glm::length(vf);
	vf = (offset * (offset < 0.0 ? -offset : offset) * kf / slen) * glm::normalize(vf);

	sph[p].speed += vf * (dT / sph[p].mass);
	sph[s].speed -= vf * (dT / sph[s].mass);
} */

// Viscous struts
void vstrut(uint16_t s, uint16_t p, double slen, double& dT)
{
	glm::dvec3 vf = sph[p].coords - sph[s].coords;
	float vcos = ph::sp::cosvec3(vf, sph[p].speed - sph[s].speed);
	double offset = slen - glm::length(vf);
	vf = (offset * (offset < 0.0 ? -offset : offset) * kf / slen) * glm::normalize(vf);
	
	if (vcos < 0.f xor offset < 0.0)
		vf *= (1.f + visc);
	else vf *= (1.f - visc);
	
	sph[p].speed += vf * (dT / sph[p].mass);
	sph[s].speed -= vf * (dT / sph[s].mass);
}

void vstrut(uint16_t s, glm::dvec3 p, double slen, double& dT, float h = kf)
{
	glm::dvec3 vf = p - sph[s].coords;
	float vcos = ph::sp::cosvec3(vf, -sph[s].speed);
	double offset = slen - glm::length(vf);
	vf = (offset * (offset < 0.0 ? -offset : offset) * h / slen) * glm::normalize(vf);
	
	if (vcos < 0.f xor offset < 0.0)
		vf *= (1.f + visc);
	else vf *= (1.f - visc);
	
	sph[s].speed -= vf * (dT / sph[s].mass);
}

static void sphUpdatePhysics(double& dT)
{
	for (uint16_t n = 0U; n < sph.size(); ++n)
	{
		glm::dvec3 force_local;
		force_local = ph::gravitacc(sph[n].coords, glm::dvec3(0.0), g_sunMass) * (double)sph[n].mass;
		if (!ph::elenvec3(sph[n].coords, sph[n].r + g_sunRadius)) {
			double offset = ph::getDist() - sph[n].r - g_sunRadius;
			if (glm::length(sph[n].speed * dT + sph[n].coords) < ph::getDist())
				force_local += offset * offset * kf * (1.0 + visc) * glm::normalize(sph[n].coords);
			else force_local += offset * offset * kf * (1.0 - visc) * glm::normalize(sph[n].coords);
		}
		for (uint16_t i = 0U; i < sph.size(); ++i) // collide with other spheres
		{ 
			glm::dvec3 ptr = sph[n].coords - sph[i].coords;
			if ((i != n) && !ph::elenvec3(ptr, sph[n].r + sph[i].r)) {
				double offset = ph::getDist() - sph[n].r - sph[i].r;
				if (glm::length((sph[n].speed - sph[i].speed) * dT + sph[n].coords - sph[i].coords) < ph::getDist())
					force_local += offset * offset * kf * (1.0 + visc) * glm::normalize(ptr);
				else force_local += offset * offset * kf * (1.0 - visc) * glm::normalize(ptr);
			}
		}
		if (input::mMods & (1 << GLFW_MOUSE_BUTTON_RIGHT))
		{
			if (!ph::elenvec3(camera.WorldPos + (glm::dvec3)camera.Forward - sph[n].coords, 8.0))
			{
				if (!sph[n].picked || input::mMods & (1 << GLFW_MOUSE_BUTTON_LEFT))
					sph[n].speed = camera.camV;
			
				sph[n].picked = true;
				vstrut(n,
					camera.WorldPos + (glm::dvec3)(5.f * camera.Forward),
					0.8660254037, dT, 2450.f);
			}
		}
		else
		{
			sph[n].picked = false;
			if (input::mMods & (1 << GLFW_MOUSE_BUTTON_LEFT))
			{
				if (!ph::elenvec3(camera.WorldPos + (glm::dvec3)camera.Forward - sph[n].coords, 8.0))
					sph[n].speed += (glm::dvec3)camera.Forward * (dT * 2500.0 / sph[n].mass);
			}
		}
		
		sph[n].speed += (dT / sph[n].mass) * force_local;
		sph[n].coords += sph[n].speed * dT;
	}
	if (app::enableStruts)
		app::putStruts(dT);
}

static void processInput(GLFWwindow* window)
{
	static bool phready = false;
	currentTime = glfwGetTime();
	if (phready) {
		deltaTime = currentTime - lastTime;
	} else {
		deltaTime = 0.0;
		phready = currentTime > 1.f;
	}
	lastTime = currentTime;

	double dTime = deltaTime / tdiv;
	for (uint16_t k = 0; k < tdiv; ++k) {
		sphUpdatePhysics(dTime);
		cube.updatePhysics(dTime);
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	int cmov = 0;

	if (glfwGetKey(window, GLFW_KEY_W))
		cmov = camera.MOV_FW;
	else if (glfwGetKey(window, GLFW_KEY_S))
		cmov = camera.MOV_BW;
	if (glfwGetKey(window, GLFW_KEY_A))
		cmov |= camera.MOV_L;
	else if (glfwGetKey(window, GLFW_KEY_D))
		cmov |= camera.MOV_R;
	if (glfwGetKey(window, GLFW_KEY_SPACE))
		cmov |= camera.MOV_UP;
	else if (input::kMods & GLFW_MOD_SHIFT)
		cmov |= camera.MOV_DN;
	
	camera.processKeyboard(cmov, deltaTime);
	camera.updatePhysics(deltaTime);

	if (glfwGetKey(window, GLFW_KEY_R))
	{
		// cube.coords = glm::dvec3(140.0, 0.0, 0.0);
		// cube.speed = glm::dvec3(0.0, 0.0, 0.0);
		
		app::resetScene();

		for (uint8_t i = 0U; i < sph.size(); i++)
			sph[i].speed = glm::dvec3(0.0, 0.0, 0.0);
		// cube.rotateVec = glm::vec3(0.f, 0.00001, 0.f);
		// cube.rvn = glm::vec3(0.f, 1.f, 0.f);
		// cube.rotateRads = 0.f;
	}

	if (glfwGetKey(window, GLFW_KEY_V)) {
		glm::dvec3 force = glm::cross(sph[1].coords - sph[0].coords, sph[2].coords - sph[0].coords) *  deltaTime;
		sph[1].speed += force / (double)sph[1].mass;
		sph[3].speed -= force / (double)sph[3].mass; 
	}

	/* if (key_delay > 0.f)
		key_delay -= deltaTime;
	
	if (key_delay < 0.f)
		key_delay = 0.f; */
}

static void keyboard102(GLFWwindow* window, int key, int scancode, int action, int mods) // GLFW callback function
{
	input::kMods = *(uint8_t*)&mods; // copy mods to kMods bit by bit

	if (action != GLFW_RELEASE) {
		if (key == GLFW_KEY_E)
			app::enableStruts = !app::enableStruts;
		else if (key == GLFW_KEY_Q)
		{
			if (app::sphState < 2)
				++app::sphState;
			else app::sphState = 0;
			app::resetScene();
		}
		else if (key == GLFW_KEY_T)
			visc += 0.05f;
		else if (key == GLFW_KEY_G && visc > 0.05f)
			visc -= 0.05f;
		else if (key == GLFW_KEY_U)
			kf *= 1.1f;
		else if (key == GLFW_KEY_H)
			kf /= 1.1f;
		else if (key == GLFW_KEY_M)
			g_sunMass *= 1.1f;
		else if (key == GLFW_KEY_N)
			g_sunMass /= 1.1f;
		else if (key == GLFW_KEY_P && sph.size() > 0U)
			sph.pop_back();
		else if (key == GLFW_KEY_O) // Add a sphere
		{
			uint16_t n = sph.size();
			sph.resize(n + 1);
			sph[n].r = (n > 7) || (app::sphState != 2) ? 1.f : 0.f;
			sph[n].mass = 5.f;
			sph[n].coords = glm::dvec3(150., 0., 0.);
		}
		else if (key == GLFW_KEY_B) {
			// system("cls");
			logfile.open("latest.log", std::fstream::app);
			logfile << glfwGetTime() << "s:\nkf == " << kf <<
				"\nvisc == " << visc <<
				"\ntdiv == " << tdiv <<
				"\ncube.r == " << cube.r << 
				"\ncamera.WorldPos == ";
			writeVector(logfile, camera.WorldPos);
			writeVector(logfile, camera.Forward);
			writeVector(logfile, camera.Up);
			logfile << GLFW_MOUSE_BUTTON_LEFT << ' '
				 << GLFW_MOUSE_BUTTON_RIGHT << ' '
				 << GLFW_MOUSE_BUTTON_MIDDLE << '\n';
			logfile.close();
		}
		else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
			if (graphics::skybox & 2) graphics::skybox ^= 1;
			else // if (skybox == 0)
			{ // retry loading cubemap
				stbi_set_flip_vertically_on_load(false);
				graphics::loadCubemap(&graphics::skybox);
				if (graphics::skybox) {
					skyboxShader.structShaders("../vs/skybox.glsl", "../fs/skybox.glsl");
					skyboxShader.use();
					skyboxShader.setInt("cubeMap", 4);
				}
			}
		}
	}
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	graphics::WinW = width; graphics::WinH = height;
	graphics::flags |= GR_VIEW_CHANGE;
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action != GLFW_RELEASE)
		{
			input::mMods |= (1 << GLFW_MOUSE_BUTTON_LEFT);
			if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				camera.fInput |= camera.M_INIT;
			}
		}
		else input::mMods &= ~(1 << GLFW_MOUSE_BUTTON_LEFT);
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action != GLFW_RELEASE)
			input::mMods |= (1 << GLFW_MOUSE_BUTTON_RIGHT);
		else input::mMods &= ~(1 << GLFW_MOUSE_BUTTON_RIGHT);
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		if (action != GLFW_RELEASE)
		{
			input::mMods |= (1 << GLFW_MOUSE_BUTTON_MIDDLE);
			camera.RenderDist = 500.f;
			camera.fInput |= camera.VIEW_CHANGE;
		}
		else input::mMods &= ~(1 << GLFW_MOUSE_BUTTON_MIDDLE);
	}
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		camera.processMouseMov(xpos, ypos);
}

static void mouse_scroll_callback(GLFWwindow* window, double, double yoffset)
{
	camera.scrollCallback(yoffset, input::kMods);
}

namespace graphics
{
	static void initPlanet()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(sphVerts), sphVerts, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphIndices), sphIndices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	static void initSph()
	{
		glGenVertexArrays(1, &sphVAO);
		glGenBuffers(1, &sphVBO);
		glGenBuffers(1, &sphEBO);

		glBindVertexArray(sphVAO);

		glBindBuffer(GL_ARRAY_BUFFER, sphVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(sphsVerts), sphsVerts, GL_STATIC_DRAW);

		//delete[] sphmVerts;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphsIndices), sphsIndices, GL_STATIC_DRAW);

		//delete[] sphmInd;

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	static void initCube()
	{
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glGenBuffers(1, &cubeEBO);
		
		glBindVertexArray(cubeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
	static void initSkybox()
	{
		#include "skybox.h"
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glGenBuffers(1, &skyboxEBO);
		
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	static int Init()
	{
		int State;
		logfile.open("latest.log", std::fstream::trunc);
		State = glfwInit();
		logfile << glfwGetTime() << "s: glfwInit() == " << State << '\n';
		logfile.close();
		if (!State)
			return -1;
		
		const GLFWvidmode* vm = glfwGetVideoMode(glfwGetPrimaryMonitor());
		WinW = vm->width;
		WinH = vm->height;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_REFRESH_RATE, vm->refreshRate);

	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

		window = glfwCreateWindow(WinW, WinH, "CTest 0.3.0 alpha", NULL, NULL);
		if (!window)
		{
			logfile.open("latest.log", std::fstream::app);
			logfile << "GLFW: failed to create window\n";
			logfile.close();
			glfwTerminate();
			return -1;
		}

		glfwMakeContextCurrent(window);
		glfwSetKeyCallback(window, keyboard102);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetCursorPosCallback(window, cursor_pos_callback);
		glfwSetScrollCallback(window, mouse_scroll_callback);
		glfwSwapInterval(1);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		State = gladLoaderLoadGL(); // glad 2.0.0
		logfile.open("latest.log", std::fstream::app);
		logfile << glfwGetTime() << "s: gladLoaderLoadGL() == " << State << '\n';
		logfile.close();
		if (!State)
		{
			glfwTerminate();
			return -1;
		}
		glEnable(GL_DEPTH_TEST);
		
		initPlanet();
		initSph();
		initCube();
		// initSkybox(); // called by loadCubemap
		
		stbi_set_flip_vertically_on_load(true);
		loadTexture("textures/board.png", &texture0);
		loadTexture("textures/test.png", &texture1);
		loadTexture("textures/block.png", &texture2);
		loadTexture("textures/worldmap.jpg", &texture3);
		
		skybox = 0;
		stbi_set_flip_vertically_on_load(false);
		loadCubemap(&skybox);
		
		return 0;
	}
	static void loadTexture(const char* path, unsigned int* textureID)
	{
		glGenTextures(1, textureID);

		int width, height, nrComponents;
		stbi_uc* data = stbi_load(path, &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;
			else format = GL_RED;

			glBindTexture(GL_TEXTURE_2D, *textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			logfile.open("latest.log", std::fstream::app);
			logfile << "Texture failed to load: " << path << '\n';
			logfile.close();
		}
		stbi_image_free(data);
	}
	static void loadCubemap(char* skyTexture)
	{
		char** faces = new char* [6];
		faces[0] = (char*)"textures/bkg1_right.png";
		faces[1] = (char*)"textures/bkg1_left.png";
		faces[2] = (char*)"textures/bkg1_top.png";
		faces[3] = (char*)"textures/bkg1_bot.png";
		faces[4] = (char*)"textures/bkg1_back.png";
		faces[5] = (char*)"textures/bkg1_front.png";

		if (*skyTexture == 0) {
			initSkybox();
			glGenTextures(1, &cubeMap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
			*skyTexture = SKYBOX_INIT;
		}

		int width, height, nrChannels;
		for (unsigned int i = 0; *skyTexture && i < 6; ++i)
		{
			stbi_uc* data = stbi_load(faces[i], &width, &height, &nrChannels, 0); 
			if (data)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			else {
				logfile.open("latest.log", std::fstream::app);
				logfile << "Cubemap texture failed to load at path: " << faces[i] << '\n';
				logfile.close();
				*skyTexture = 0;
			}
			stbi_image_free(data);
		}
		if (*skyTexture) {
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
		} else {
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glDeleteTextures(1, &cubeMap);
			glDeleteVertexArrays(1, &skyboxVAO);
			glDeleteBuffers(1, &skyboxVBO);
		}
	}
	static void display()
	{
		static glm::mat4 projection;
		if ((graphics::flags & GR_VIEW_CHANGE)
		||	(camera.fInput & camera.VIEW_CHANGE))
		{
			projection = glm::perspective(camera.FoV, (float)graphics::WinW / graphics::WinH, 0.1f, camera.RenderDist);
			camera.fInput &= ~(camera.VIEW_CHANGE);
			graphics::flags &= ~GR_VIEW_CHANGE;
		}

		// Render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texture3);

		glm::mat4 view = camera.getViewMatrix();
		
		{
			glBindVertexArray(sphVAO);
			sphShader.use();
			sphShader.setMat4("view", view);
			sphShader.setMat4("projection", projection);
			
			for (uint16_t n = (app::sphState == 2) ? 8 : 0; n < sph.size(); ++n)
			{
				glm::mat4 model = glm::mat4(1.f);
				model = glm::translate(model, (glm::vec3)sph[n].coords);
				model = glm::scale(model, glm::vec3(1.f) * sph[n].r);
				sphShader.setMat4("model", model);
				glDrawElements(GL_TRIANGLES, sizeof(sphsIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
			}
		}

		sunShader.use();
		sunShader.setMat4("view", view);
		sunShader.setMat4("projection", projection);
		
		{
			glBindVertexArray(VAO);
			glm::mat4 model(1.f);
			sunShader.setMat4("model", model);
			glDrawElements(GL_TRIANGLES, sizeof(sphIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
		}
		
		if (app::sphState == 2)
		{
			glBindVertexArray(cubeVAO);
			
			cubeShader.use();
			cubeShader.setMat4("view", view);
			cubeShader.setMat4("projection", projection);
			glm::mat4 model(1.f);
			
			glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			glBufferData(GL_ARRAY_BUFFER, cube.surface_len * sizeof(float), cube.surface, GL_DYNAMIC_DRAW);
			model = glm::translate(model, (glm::vec3)cube.coords);
			//model = scale(model, sunScale);
			cubeShader.setMat4("model", model);
			glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
		}


		if (skybox == 3)
		{
			skyboxShader.use();
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
			// draw skybox as last
			glDepthFunc(GL_LEQUAL);
			skyboxShader.setMat4("view", glm::mat4(glm::mat3(view))); // remove translation from the view matrix
			skyboxShader.setMat4("projection", projection);
			// skybox cube
			glBindVertexArray(skyboxVAO);
			// glDrawArrays(GL_TRIANGLES, 0, 36);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS); // set depth function back to default
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
} // namespace graphics

namespace app
{
	static void reset_spheres()
	{
		sphState = 0;
		sph.resize(0);
		kf = 7.e5f;
	}

	static void rotative_figure_form()
	{
		sphState = 1;
		sph.resize(6);
		for (uint8_t a = 0; a < 6; ++a) {
			sph[a].r = 1.f;
		}
		
		sph[0].mass = 20.f;
		sph[1].mass = 20.3f;
		sph[2].mass = 20.f;
		sph[3].mass = 20.3f;
		
		sph[0].coords = glm::dvec3(125., 0., 70.);
		sph[1].coords = glm::dvec3(125., 15., 70.);
		sph[2].coords = glm::dvec3(140., 15., 70.);
		sph[3].coords = glm::dvec3(140.0, 0., 70.);
		sph[4].coords = glm::dvec3(132.5, 7.5, 75.);
		sph[5].coords = glm::dvec3(132.5, 7.5, 65.);
		
		g_sunMass = 25.e10;
		kf = 1.5e11f;
	}

	static void cube_form()
	{
		sphState = 2;
		sph.resize(8);
		for (uint8_t a = 0; a < 8; ++a) {
			sph[a].r = 0.0f;
			sph[a].mass = 5.f;
		}
		
		sph[0].coords = glm::dvec3(125., 0., 70.);
		sph[1].coords = glm::dvec3(126., 0., 70.);
		sph[2].coords = glm::dvec3(126., 1., 70.);
		sph[3].coords = glm::dvec3(125., 1., 70.);
		sph[4].coords = glm::dvec3(125., 0., 71.);
		sph[5].coords = glm::dvec3(126., 0., 71.);
		sph[6].coords = glm::dvec3(126., 1., 71.);
		sph[7].coords = glm::dvec3(125., 1., 71.);
		
		g_sunMass = 25.e14;
		kf = 3.5e7f;
	}
	
	void resetScene()
	{
		if (sphState == 0)
			reset_spheres();
		else if (sphState == 1)
			rotative_figure_form();
		else cube_form();
	}
		
	
	void putStruts(double& time)
	{
		if (sphState == 1)
		{
			vstrut(0, 1, 15.0, time);
			vstrut(1, 2, 15.0, time);
			vstrut(2, 3, 15.0, time);
			vstrut(0, 3, 15.0, time);
			vstrut(0, 2, 21.2132034355964, time);
			vstrut(1, 3, 21.2132034355964, time);

			vstrut(4, 5, 10.f, time);

			for (uint8_t o = 0; o < 4; ++o) {
				vstrut(o, 4, 11.7260394, time);
				vstrut(o, 5, 11.7260394, time);
			}
		}
		else if (sphState == 2)
		{
			vstrut(0, 1, 1.0, time);
			vstrut(1, 2, 1.0, time);
			vstrut(2, 3, 1.0, time);
			vstrut(0, 3, 1.0, time);
			vstrut(0, 2, 1.414213562373, time);
			vstrut(1, 3, 1.414213562373, time);
			
			
			vstrut(4, 5, 1.0, time);
			vstrut(5, 6, 1.0, time);
			vstrut(6, 7, 1.0, time);
			vstrut(4, 7, 1.0, time);
			vstrut(4, 6, 1.414213562373, time);
			vstrut(5, 7, 1.414213562373, time);
			
			for (uint8_t o = 0; o < 4; ++o)
				vstrut(o, o + 4, 1.0, time);
			
			vstrut(0, 5, 1.414213562373, time);
			vstrut(1, 4, 1.414213562373, time);
			vstrut(2, 7, 1.414213562373, time);
			vstrut(3, 6, 1.414213562373, time);
			
			vstrut(1, 6, 1.414213562373, time);
			vstrut(2, 5, 1.414213562373, time);
			vstrut(0, 7, 1.414213562373, time);
			vstrut(3, 4, 1.414213562373, time);
		}
	}
	
} // namespace app
