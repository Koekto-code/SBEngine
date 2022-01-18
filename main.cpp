// 18 january 2022
// ctest 0.2.2

/*#if defined(_MSC_VER)
// Make MS math.h define M_PI
#define _USE_MATH_DEFINES
#endif*/

#include <iostream>
//#include <fstream>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

//#include <sph.hpp>
#include "physics.h"
#include "misc.h"

//#include "gltext.cpp"
//#include "dtoa.cpp"
#include "shader.hpp"
#include "cube.h"
#include "sphere.h"
#include "sphere_m.h"


static void processInput(GLFWwindow* window);
static void keyboard102(GLFWwindow* window, int key, int scancode, int action, int mods);
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
static void mouse_scroll_callback(GLFWwindow* window, double, double yoffset);

static void loadCubemap(char** faces);

static char k_mods; // shift control alt etc (GLFW input)
static char skybox = 3Ui8; // sky texture: 3- display; 2- available, don't display; 0- not available

static uint8_t mouseState = 0Ui8; // all buttons released

static unsigned int VBO, EBO, VAO, cubeVBO, cubeEBO, cubeVAO, sphVBO, sphEBO, sphVAO, skyboxVBO, skyboxVAO; // OpenGL vertex buffers objects, element buffer objects & vertex array objects
static unsigned int texture0, texture1, texture2, texture3, cubeMap;

static unsigned int SCR_WIDTH = 1366U;
static unsigned int SCR_HEIGHT = 768U;
static GLFWwindow* window;

static glm::dvec3 cameraPos(140.0, 0.0, 15.0);
static glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // unit vector (doesn't change in this version)
//static glm::vec3 cameraRight = glm::vec3(1.f, 0.f, 0.f); // unit vector
static glm::vec3 cameraForward = glm::vec3(0.0f, 0.0f, -1.0f); // unit vector
static glm::vec3 cameraSpeed = glm::vec3(0.f, 0.f, 0.f);

static float sensitivity = 0.1f; // of changing cameraForward by moving the mouse
static float renderDist = 500.f; // metres
static bool mouseinit = true; // when cursor has just been disabled by GLFW
static float lastX = 400.0f; // does not matter
static float lastY = 300.0f; // does not matter
static float fov = 45.0f; // prefered angle between (top display border -- eyes -- bottom display border)
static bool turnfov = true; // whenever FOV changing

static float yaw = -90.0f; // for camera
static float pitch = 0.0f; // for camera

static double currentTime = 0.0; // returned by GLFW
static double lastTime = 0.0;
static double deltaTime = 0.0;


static double tdiv = 177.0; // time divider for spheres & other objects
static float key_delay = 0.f; // keyboard delay


struct Sphere {
	float r = 2.f;
	float mass = 5.f;
	glm::dvec3 coords; 
	glm::dvec3 speed = glm::dvec3(0., 0., 0.);
};

static Shader sunShader; // for the main sphere
static Shader sphShader; // for small spheres
static Shader cubeShader;
static Shader skyboxShader; 

static float kf = 5e5f; // spheres' hardness
static float visc = 0.1f; // spheres' viscosity (up to 1)

std::vector<Sphere> sph;

static float sunMass = 500000.f; // not kg (const G is replaced by 1 for less calculating)
static float sunRadius = 125.f; // metres
static glm::vec3 sunScale = glm::vec3(sunRadius) / 25.f; // generated main sphere has radius 25 m, it was not enough

static float ph_model[sizeof(cubeVerts) / sizeof(float)]; // cube vertices

class Object // needs revision!
{
public:
	float r; // maximum of vertice distance from center of gravity

	glm::dvec3 coords; // world position
	glm::dvec3 speed = glm::dvec3(0.0, 0.0, 0.0);

	glm::vec3 rotateVec = glm::vec3(1.f, 0.57445626f, 0.57445626f); // axis of rotation (not unit vector)
	float rotateRads = 0.f; // angular position

	float* init_surface; // model (static)
	float* surface; // model (rotating)
	
	uint32_t surface_len; // sizeof(array_that_is_used_by_surface) / sizeof(float)
	uint8_t stride; // = 5 for (coords + tex_coords) models. All models in this version are such

	void set_mass(float m/*, glm::vec3* ptr = nullptr, uint8_t q = 1Ui8*/)
	{
		mass = m;
		/*if (ptr)
		{
			masspts = ptr;
			mpts = q;
		}
		m_unit = m / mpts;*/
	}

	void updatePhysics(double delta) // deltaTime for this
	{
		speed += ph_gravitacc(coords, glm::dvec3(0., 0., 0.), sunMass) * delta;
		coords += speed * delta;

		if (!ph_exclength(glm::dvec3(0.0, 0.0, 0.0), coords, sunRadius + r))
		{
			for (uint8_t c = 0Ui8; c < 8Ui8; c++)
			{
				glm::vec3 vert(surface[c*5Ui8], surface[c*5Ui8+1Ui8], surface[c*5Ui8+2Ui8]); // get x, y, z
				vert += coords;
				float offset = sunRadius - glm::length(vert);
				if (offset > 0.f)
				{
					glm::vec3 force;
					if (length(speed * delta + coords) < glm::length(coords))
						force = (offset * offset * kf * (1.f + visc)) * glm::normalize(vert);
					else force = (offset * offset * kf * (1.f - visc)) * glm::normalize(vert);
					speed += (glm::dvec3)force * (delta / mass);
				}
			}
		}
		if (rotateVec != glm::vec3(0.f, 0.f, 0.f)) { // otherwise it gets NaN vector
			rvec_normalized = glm::normalize(rotateVec);
			rotateRads += glm::length(rotateVec) * (float)delta / glm::length(rvec_normalized);

			if (rotateRads > glm::two_pi<double>())
				rotateRads -= glm::two_pi<double>();
			else if (rotateRads < -glm::two_pi<double>())
				rotateRads += glm::two_pi<double>();
			
			for (uint16_t i = 0; i < sizeof(ph_model) / 20Ui8; i++)
			{
				glm::vec3 rotatif = ph_rotatef(rvec_normalized, glm::vec3(cubeVerts[i * 5U], cubeVerts[i * 5U + 1U], cubeVerts[i * 5U + 2U]), rotateRads);

				ph_model[i * 5U] = rotatif.x;
				ph_model[i * 5U + 1U] = rotatif.y;
				ph_model[i * 5U + 2U] = rotatif.z;
			}
		}
	}
private:
	float mass;
	//glm::vec3* masspts; // coords about gravity center
	//uint8_t mpts = 0Ui8; // quantity of mass points
	//float m_unit; // mass of each mass point

	glm::vec3 rvec_normalized;
};

Object cube;

/*static double sqr(double a) {
	return a * a;
}

static float sqr(float a) {
	return a * a;
}*/

static void init_sun_graphics()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphVerts), sphVerts, GL_STATIC_DRAW);

	//delete[] sphVerts;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphIndices), sphIndices, GL_STATIC_DRAW);

	//delete[] sphIndices;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5U * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5U * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

static void init_sph_graphics()
{
	glGenVertexArrays(1, &sphVAO);
	glGenBuffers(1, &sphVBO);
	glGenBuffers(1, &sphEBO);

	glBindVertexArray(sphVAO);

	glBindBuffer(GL_ARRAY_BUFFER, sphVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphVerts_m), sphVerts_m, GL_STATIC_DRAW);

	//delete[] sphVerts_m;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphIndices_m), sphIndices_m, GL_STATIC_DRAW);

	//delete[] sphIndices_m;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5U * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5U * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

static void init_cube_graphics()
{
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);

	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5U * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5U * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

static void init_skybox_graphics()
{
#include "skybox.c"
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

static void load_texture0() // Wooden box
{

	glGenTextures(1, &texture0);
	glBindTexture(GL_TEXTURE_2D, texture0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("textures/board.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load textures/board.png\n";

	stbi_image_free(data);
}

static void load_texture1() // texture that is mixed with board.png
{
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load("textures/test.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load textures/test.png\n";

	stbi_image_free(data);
}

static void load_texture2() // the cube
{
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_uc* data = stbi_load("textures/block.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load textures/block.png\n";

	stbi_image_free(data);
}

static void load_texture3() // worldmap
{
	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_2D, texture3);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load("textures/worldmap.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load textures/worldmap.png\n";

	stbi_image_free(data);
}

static void reset_spheres()
{
	sph[0].coords = glm::dvec3(125., 0., 70.);
	sph[1].coords = glm::dvec3(125., 15., 70.);
	sph[2].coords = glm::dvec3(140., 15., 70.);
	sph[3].coords = glm::dvec3(125.0, 15.0, 85.);
	sph[4].coords = glm::dvec3(132.5, 7.5, 75.);
	sph[5].coords = glm::dvec3(132.5, 7.5, 65.);
	sph[6].coords = glm::dvec3(140., 15., 55.);
	sph[7].coords = glm::dvec3(140., 0., 55.);
	sph[8].coords = glm::dvec3(140., 7.5, 55.0);
}

static void display()
{
	static glm::mat4 projection;
	if (turnfov) {
		projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, renderDist);
		turnfov = false;
	}

	sunShader.use();
	sunShader.setMat4("projection", projection);
	sphShader.use();
	sphShader.setMat4("projection", projection);
	cubeShader.use();
	cubeShader.setMat4("projection", projection);


	processInput(window); 

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
	

	glm::mat4 view = lookAt(cameraPos, cameraPos + (glm::dvec3)cameraForward, (glm::dvec3)cameraUp);

	cubeShader.setMat4("view", view);

	sphShader.use();
	sphShader.setMat4("view", view);

	glBindVertexArray(sphVAO);

	for (uint16_t n = 0; n < sph.size(); n++) {
		glm::mat4 model = glm::mat4(1.f);
		model = glm::translate(model, (glm::vec3)sph[n].coords);
		sphShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, sizeof(sphIndices_m) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
	}

	sunShader.use();
	sunShader.setMat4("view", view);

	glBindVertexArray(VAO);

	{
		glm::mat4 model(1.f);
		model = scale(model, sunScale);
		sunShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, sizeof(sphIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
	}

	cubeShader.use();
	glBindVertexArray(cubeVAO);

	{
		glm::dmat4 model(1.);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ph_model), ph_model, GL_DYNAMIC_DRAW);
		model = glm::translate(model, cube.coords);
		//model = scale(model, sunScale);
		cubeShader.setMat4("model", model);
		glDrawElements(GL_TRIANGLES, 36U, GL_UNSIGNED_INT, 0);
	}

	if (skybox == 3Ui8)
	{
		skyboxShader.use();
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.setMat4("view", glm::mat4(glm::mat3(view))); // remove translation from the view matrix
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default
	}

	glfwSwapBuffers(window);
	glfwPollEvents();
}

int main(int argc, char** argv)
{
	int gls = _set_SSE2_enable(1);
	if (!gls)
		std::cout << "cmath: Couldn't enable SSE2\n";

	gls = glfwInit();
	std::cout << glfwGetTime() << " seconds; glfwInit() returns " << gls << '\n';
		
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_REFRESH_RATE, glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SPH 0.2.2 alpha", NULL, NULL);
	if (!window)
	{
		std::cout << "GLFW: failed to create window\n";
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

	gls = gladLoaderLoadGL(); // glad 2.0.0
	std::cout << glfwGetTime() << " seconds; gladLoaderLoadGL() returns " << gls << '\n';

	glEnable(GL_DEPTH_TEST);

	sph.resize(9U);
	for (unsigned a = 0U; a < 9U; a++)
	{
		sph[a].r = 1.f;
		sph[a].mass = 5.f;
	}
	/*sph[0].mass = 20.f;
	sph[1].mass = 20.3f;
	sph[2].mass = 20.f;
	sph[3].mass = 20.3f;
	sph[4].mass = sph[5].mass = 0.2f;*/

	{
		cube.set_mass(5.f/*, &massPoints[0], sizeof(massPoints) / sizeof(glm::vec3)*/);
		cube.coords = glm::dvec3(140.0, 0.0, 0.0);
		cube.init_surface = &cubeVerts[0];
		cube.surface = &ph_model[0];
		cube.surface_len = sizeof(ph_model) >> 2U;
		cube.stride = 5Ui8;
		/*cube.r = 0.f;
		for (uint16_t i = 0Ui16; i < cube.surface_len; i += 5Ui16)
		{
			float len = glm::length(vec3(cubeVerts[i], cubeVerts[i + 1Ui16], cubeVerts[i + 2Ui16]));
			if (len > cube.r)
				cube.r = len;
		}*/
		cube.r = 0.8660254f;

		for (uint16_t i = 0Ui16; i < sizeof(cubeVerts) >> 2Ui16; i++)
			ph_model[i] = cubeVerts[i];
	}

	reset_spheres();

	init_sun_graphics();
	init_sph_graphics();
	init_cube_graphics();
	init_skybox_graphics();

	glBindVertexArray(0);

	load_texture0();
	load_texture1();
	load_texture2();
	load_texture3();

	char** faces = new char*[6];
	faces[0] = (char*)"textures/bkg1_right.png";
	faces[1] = (char*)"textures/bkg1_left.png";
	faces[2] = (char*)"textures/bkg1_top.png";
	faces[3] = (char*)"textures/bkg1_bot.png";
	faces[4] = (char*)"textures/bkg1_back.png";
	faces[5] = (char*)"textures/bkg1_front.png";
	loadCubemap(faces);

	sunShader.struct_shaders("vs_sun.glsl", "fs_sun.glsl");
	sphShader.struct_shaders("vs_sphere.glsl", "fs_sphere.glsl");
	cubeShader.struct_shaders("vs.glsl", "fs.glsl");

	// numbers of textures must be different
	sunShader.use();
	sunShader.setInt("texture3", 3);

	sphShader.use();
	sphShader.setInt("texture0", 0);
	sphShader.setInt("texture1", 1);

	cubeShader.use();
	cubeShader.setInt("texture2", 2);
	//

	if (skybox) {
		skyboxShader.struct_shaders("vs_skybox.glsl", "fs_skybox.glsl");
		skyboxShader.use();
		skyboxShader.setInt("cubeMap", 4);
	}

	glClearColor(0.678f, 0.784f, 0.898f, 1.f); // color of sky

	while (!glfwWindowShouldClose(window))
		display();

	// Optional: free all the resources
	glDeleteVertexArrays(1u, &VAO);
	glDeleteBuffers(1u, &VBO);
	glDeleteBuffers(1u, &EBO);
	glDeleteVertexArrays(1u, &cubeVAO);
	glDeleteBuffers(1u, &cubeVBO);
	glDeleteBuffers(1u, &cubeEBO);
	glDeleteVertexArrays(1u, &sphVAO);
	glDeleteBuffers(1u, &sphVBO);
	glDeleteBuffers(1u, &sphEBO);
	if (skybox) {
		glDeleteVertexArrays(1U, &skyboxVAO);
		glDeleteBuffers(1U, &skyboxVBO);
		skybox = false;
	}

	glfwTerminate();
}

void strut(uint16_t s, uint16_t p, float dist, double* delta)
// Imitation of abolutely hard strut
{
	glm::dvec3 rcoords = sph[p].coords - sph[s].coords;
	glm::dvec3 tc = sph[s].coords;

	sph[p].coords += sph[p].speed * (*delta);
	sph[s].coords += sph[s].coords * (*delta);
	glm::dvec3 ptr = sph[p].coords - sph[s].coords;
	glm::dvec3 sm = ptr - (double)dist * glm::normalize(ptr);
	
	sm *= (0.5 / (*delta));
	sph[s].coords = tc;
	sph[p].coords = tc + (glm::dvec3)rcoords;
	sph[s].speed += sm;
	sph[p].speed -= sm;
}

void doStruts(double &time)
{
	strut(0, 1, 15.f, &time);
	strut(1, 2, 15.0, &time);
	strut(0, 2, 21.2132034355964f, &time);
	strut(1, 3, 15.f, &time);
	strut(2, 3, 21.2132034355964f, &time);
	strut(0, 3, 21.2132034355964f, &time);

	//strut(4, 5, 10.0, &time);

	//for (uint8_t o = 0Ui8; o < 4Ui8; o++) {
	//	strut(o, 4, 11.726039399558573886414075283861, &time);
	//	strut(o, 5, 11.726039399558573886414075283861, &time);
	//}

	//strut(6, 7, 15.0, &time);
	//strut(7, 8, 7.5, &time);
	//strut(6, 8, 7.5, &time);
}

static void sphUpdatePhysics(double time) {
	for (uint16_t n = 0Ui16; n < sph.size(); n++)
	{
		glm::dvec3 force_local;
		force_local = ph_gravitacc(sph[n].coords, glm::dvec3(0., 0., 0.), sunMass) * (double)sph[n].mass;
		if (!ph_exclength(glm::dvec3(0., 0., 0.), sph[n].coords, sph[n].r + sunRadius)) {
			double temp_offset = ph_rad() - sph[n].r - sunRadius;
			if (length(sph[n].speed * time + sph[n].coords) < ph_rad())
				force_local += temp_offset * temp_offset * kf * (1.0 + visc) * glm::normalize(sph[n].coords);
			else force_local += temp_offset * temp_offset * kf * (1.0 - visc) * glm::normalize(sph[n].coords);
		}
		if (!ph_exclength(cameraPos, sph[n].coords, sph[n].r + 0.5)) {
			double temp_offset = ph_rad() - sph[n].r - 0.5;
			if (length(sph[n].speed * time + sph[n].coords - cameraPos) < ph_rad())
				force_local += temp_offset * temp_offset * kf * (1.0 + visc) * glm::normalize(sph[n].coords - cameraPos);
			else force_local += temp_offset * temp_offset * kf * (1.0 - visc) * glm::normalize(sph[n].coords - cameraPos);
		}
		for (uint16_t i = 0Ui16; i < sph.size(); i++) // collision the (n) sphere with other spheres from array
		{
			if ((i != n) && !ph_exclength(sph[n].coords, sph[i].coords, sph[n].r + sph[i].r)) {
				double temp_offset = ph_rad() - sph[n].r - sph[i].r;
				if (length((sph[n].speed - sph[i].speed) * time + sph[n].coords - sph[i].coords) < ph_rad())
					force_local += temp_offset * temp_offset * kf * (1.0 + visc) * glm::normalize(sph[n].coords - sph[i].coords);
				else force_local += temp_offset * temp_offset * kf * (1.0 - visc) * glm::normalize(sph[n].coords - sph[i].coords);
			}
		}
		sph[n].speed += (time / sph[n].mass) * force_local;
		sph[n].coords += sph[n].speed * time;
	}
	doStruts(time);
}

static void processInput(GLFWwindow* window)
{
	currentTime = glfwGetTime();
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	double dTime = deltaTime / tdiv;
	for (double k = 0.; k < deltaTime; k += dTime) {
		sphUpdatePhysics(dTime);
		cube.updatePhysics(dTime);
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glm::vec3 motion_acc = glm::vec3(0.f, 0.f, 0.f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		motion_acc += glm::normalize(glm::vec3(cameraForward.x, 0.f, cameraForward.z));
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		motion_acc -= glm::normalize(glm::vec3(cameraForward.x, 0.f, cameraForward.z));
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		motion_acc -= glm::normalize(cross(cameraForward, cameraUp));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		motion_acc += glm::normalize(cross(cameraForward, cameraUp));
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		motion_acc.y += 1.f;
	if (k_mods & GLFW_MOD_SHIFT)
		motion_acc.y -= 1.f;

	
	if (motion_acc != glm::vec3(0.f, 0.f, 0.f))
		cameraSpeed += (glm::dvec3)glm::normalize(motion_acc) * (deltaTime * 40.0);
	if (cameraSpeed != glm::vec3(0.f, 0.f, 0.f)) {
		glm::vec3 temporary = cameraSpeed;
		cameraSpeed -= (glm::dvec3)glm::normalize(cameraSpeed) * 4.0 * deltaTime;
		if (ph_cosvecf(temporary, cameraSpeed) < 0.5f)
			cameraSpeed = glm::vec3(0.f, 0.f, 0.f);
	}
	cameraPos += (glm::dvec3)cameraSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		cube.coords = glm::dvec3(140.0, 0.0, 0.0);
		cube.speed = glm::dvec3(0.0, 0.0, 20.0);
		reset_spheres();
		for (uint8_t i = 0Ui8; i < sph.size(); i++)
		{
			sph[i].speed = glm::dvec3(0.0, 0.0, 0.0);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		cube.speed.x += 0.1f;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		cube.speed.x -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		cube.speed.z -= 0.1f;
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		cube.speed.z += 0.1f;

	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
		glm::dvec3 force = cross(sph[1].coords - sph[0].coords, sph[2].coords - sph[0].coords) * 0.3 * deltaTime;
		sph[1].speed += force; // let the body rotate
		sph[3].speed -= force; 
	}

	for (uint8_t k = 0ui8; k < sph.size(); k++) {
		if (mouseState & 1Ui8) // if LMB is pressed
		{
			if (!ph_exclengthf(cameraPos, sph[k].coords, 8.f))
				sph[k].speed += cameraForward * (float)deltaTime * 75.f;
		}
		else if (mouseState & 2Ui8) // if RMB is pressed
		{
			if (!ph_exclengthf(cameraPos, sph[k].coords, 8.f))
				sph[k].speed += glm::normalize(cameraPos - sph[k].coords) * (deltaTime * 75.0);
		}
	}

	if (key_delay > 0.f)
		key_delay -= deltaTime;

	if (key_delay < 0.f)
		key_delay = 0.f;
}

static void keyboard102(GLFWwindow* window, int key, int scancode, int action, int mods) // other keyboard input system
// that doesn't allow to check keys each 1 ms
{
	k_mods = *(uint8_t*)&mods; // copy mods to k_mods bit by bit

	if (action != GLFW_RELEASE) { // other actions are GLFW_PRESS and GLFW_REPEAT
		if (key == GLFW_KEY_E && tdiv > .5f)
			tdiv *= 1.07;
		else if (key == GLFW_KEY_Q && tdiv > .5f)
			tdiv /= 1.07;
		else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
			visc += 0.05f;
		else if (key == GLFW_KEY_G && visc > 0.05f)
			visc -= 0.05f;
		else if (key == GLFW_KEY_U)
			kf *= 1.1f;
		else if (key == GLFW_KEY_H)
			kf /= 1.1f;
		else if (key == GLFW_KEY_M)
			sunMass *= 1.1f;
		else if (key == GLFW_KEY_N)
			sunMass /= 1.1f;
		else if (key == GLFW_KEY_P && sph.size() > 0Ui16)
			sph.resize(sph.size() - 1U);
		else if (key == GLFW_KEY_O) // Add a sphere
		{
			uint16_t n = sph.size();
			sph.resize(n + 1Ui16);
			sph[n].r = 1.f;
			sph[n].mass = 5.f;
			sph[n].coords = glm::dvec3(150., 0., 0.);
		}
		else if (key == GLFW_KEY_B) {
			system("cls");
			std::cout << "kf == " << kf <<
				"\nvisc == " << visc <<
				"\ntdiv == " << tdiv <<
				")\nyaw == " << ::yaw <<
				"\npitch == " << ::pitch <<
				"\ncameraForward == ";
			displayVector(cameraForward);
			std::cout << '\n';
		}
		else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
			if (skybox == 3Ui8)
				skybox = 2Ui8;
			else if (skybox == 2Ui8)
				skybox = 3Ui8;
		}
	}
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS) {
			if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				mouseinit = true;
			}
			mouseState |= 1Ui8;
		}
		else if (action == GLFW_RELEASE)
			mouseState &= 0xFEUi8; // 0xFE = 254
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		//renderDist += 50.f;
		//turnfov = true;
		if (action == GLFW_PRESS)
			mouseState |= 2Ui8;
		else if (action == GLFW_RELEASE)
			mouseState &= 0xFDUi8; // 0xFD = 253
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
		renderDist = 500.f;
		turnfov = true;
	}
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		if (mouseinit)
		{
			lastX = xpos;
			lastY = ypos;
			mouseinit = false;
		}

		float xoffset = (xpos - lastX) * sensitivity;
		float yoffset = (lastY - ypos) * sensitivity; // for GLFW y-axis is reversed relatively to screen coords
		lastX = xpos;
		lastY = ypos;

		::yaw += xoffset;
		::pitch += yoffset;
		
		
		if (::pitch > 89.9f)
			::pitch = 89.9f;
		else if (::pitch < -89.9f)
			::pitch = -89.9f;

		if (::yaw > 360.f)
			::yaw -= 360.f;
		else if (::yaw < -360.f)
			::yaw += 360.f;

		glm::vec3 forward;
		float cosp = cos(glm::radians(::pitch));
		forward.x = cos(glm::radians(::yaw)) * cosp;
		forward.y = sin(glm::radians(::pitch));
		forward.z = sin(glm::radians(::yaw)) * cosp;

		cameraForward = glm::normalize(forward);
	}
}

static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	turnfov = true;
	fov -= yoffset;
	fov -= xoffset * 10.f;
	if (fov < 1.0f)
		fov = 1.0f;
	else if (fov > 180.0f)
		fov = 180.0f;
}

/*unsigned int loadTexture(char const * path, unsigned int textureID)
{
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
		std::cout << "Texture failed to load at path: " << path << std::endl;
	stbi_image_free(data);

	return textureID;
}*/

static void loadCubemap(char** faces)
{
	glGenTextures(1, &cubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(false);
	for (unsigned int i = 0; skybox && (i < 6U); ++i)
	{
		unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0); 
		if (data)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else {
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			skybox = 0i8;
		}
		stbi_image_free(data);
	}
	if (skybox) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDeleteTextures(1, &cubeMap);
		glDeleteVertexArrays(1U, &skyboxVAO);
		glDeleteBuffers(1U, &skyboxVBO);
	}
}
