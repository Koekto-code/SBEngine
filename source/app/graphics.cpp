// 24 june 2022
// SBView v0.1.0-alpha

#include <app/graphics.hpp>

#include <glad/gl.h>
#include <stb_image.h>

#include <runtime/glUtil.h>
#include <runtime/shader.hpp>

#include <models/sph.h>

#include <cassert>
#include <vector>
#include <fstream>
#include <cstring>

NAMESPACE_BEGIN(graphics)

static unsigned int plVBO, plEBO, plVAO,
	skyboxVBO, skyboxEBO, skyboxVAO,
	mVBO, mVAO;
static unsigned int worldMap, skyTexture;

static Shader planetShader; // for the main sphere
static Shader stdShader; // for objects
static Shader skyShader; // custom sky
static Shader mShader; // markers

std::vector<Marker> g_markers = std::vector<Marker>(2);

static int viewChange = 0;
static int skyboxState = 0; // 3: display; 2: available, don't display; 0: not available
static int WinW, WinH;

static void initPlanet();
static void initSkybox();
static void initMarkers();
static void loadCubemap();

int getVal(grEnum query)
{
	if	(query == WIN_WIDTH)	return WinW;
	if	(query == WIN_HEIGHT)	return WinH;
	if	(query == SKYBOX_STATE)	return skyboxState;
	if	(query == VIEW_CHANGE)	return viewChange;
	return 0;
}

void resizeCallbackEvent(GLFWwindow* wnd, int width, int height)
{
	glViewport(0, 0, width, height);
	WinW = width;
	WinH = height;
	viewChange = true;
}

void toggleSkybox()
{
	// If cubemap loaded, switch it
	if (skyboxState & 2)
		skyboxState ^= 1;
	else
	{	// retry loading cubemap
		loadCubemap();
		if (skyboxState)
			applySkyboxShaders();
	}
}

void applySkyboxShaders()
{
	skyShader.compile("./shaders/skybox_vs.glsl", "./shaders/skybox_fs.glsl");
	skyShader.use();
	skyShader.setInt("skyTex", 1);
}

int init(GLFWwindow*& wnd)
{
	if (!glfwInit())
		return -101;
	
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

	wnd = glfwCreateWindow(WinW, WinH, "SBView 0.1.0 alpha", NULL, NULL);
	if (!wnd)
	{
		glfwTerminate();
		return -102;
	}
	glfwMakeContextCurrent(wnd);
	if (!gladLoaderLoadGL())
	{
		glfwTerminate();
		return -103;
	}
	
	initPlanet();
	// initSkybox(); // called by loadCubemap
	initMarkers();
	
	stbi_set_flip_vertically_on_load(true);
	loadTexture("./resources/textures/worldmap.jpg", &worldMap);
	loadCubemap();

	planetShader.compile("./shaders/planet_vs.glsl", "./shaders/planet_fs.glsl");
	stdShader.compile("./shaders/default_vs.glsl", "./shaders/default_fs.glsl");
	mShader.compile("./shaders/marker_vs.glsl", "./shaders/marker_fs.glsl");
	
	if (skyboxState)
		applySkyboxShaders();

	planetShader.use();
	planetShader.setInt("worldmap", 0);

	glClearColor(0.678f, 0.784f, 0.898f, 1.f);
	glEnable(GL_DEPTH_TEST);
	return 0;
}

void shutdown()
{
	glfwTerminate();

	glDeleteTextures(1, &worldMap);
	glDeleteBuffers(1, &plVBO);
	glDeleteBuffers(1, &plEBO);
	glDeleteVertexArrays(1, &plVAO);
	planetShader.terminate();

	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	mShader.terminate();
	
	if (skyboxState)
	{
		glDeleteTextures(1, &skyTexture);
		glDeleteBuffers(1, &skyboxVBO);
		glDeleteBuffers(1, &skyboxEBO);
		glDeleteVertexArrays(1, &skyboxVAO);
		skyShader.terminate();
		skyboxState = 0;
	}
	gladLoaderUnloadGL();
}

void loadCubemap()
{
	stbi_set_flip_vertically_on_load(false);

	char** faces = new char* [6];
	for (int i = 0; i < 6; ++i) {
		faces[i] = new char[44];
		std::strcpy(faces[i], "./resources/textures/Cubemap_Positive .png");
		if (i & 1) std::memcpy(faces[i] + 29, "Nega", 4);
		faces[i][37] = 'X' + (i >> 1);
	}

	if (!skyboxState) {
		initSkybox();
		glGenTextures(1, &skyTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);
		skyboxState = 3;
	}
	
	int width, height, nrChannels;
	for (unsigned int i = 0; skyboxState && i < 6; ++i)
	{
		stbi_uc* data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
		GLenum format;
		if (data && (format =
			nrChannels == 3 ? GL_RGB :
			nrChannels == 4 ? GL_RGBA :
			nrChannels == 1 ? GL_RED : 0)
		) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		else {
			std::ofstream logfile("latest.log", std::fstream::app);
			logfile << "Cubemap texture failed to load at path: " << faces[i] << '\n';
			logfile.close();
			skyboxState = 0;
		}
		stbi_image_free(data);
	}
	if (skyboxState)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}
	else
	{	// Free resources allocated in skyboxInit()
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDeleteTextures(1, &skyTexture);
		glDeleteVertexArrays(1, &skyboxVAO);
		glDeleteBuffers(1, &skyboxVBO);
	}
}

void display(Camera& cam, sbObject& sb_obj)
{
	if (WinW <= 0 || WinH <= 0)
		return;

	glm::mat4 projection = glm::perspective(cam.fov, (float)WinW / WinH, 0.1f, cam.renderDist);
	glm::mat4 view = cam.getViewMatrix();
	// glm::mat4 transform(1.f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, worldMap);
	
	// Display planet
	{
		planetShader.use();
		planetShader.setMat4("projection", projection);
		planetShader.setMat4("view", view);
		planetShader.setInt("worldmap", 0);
		glBindVertexArray(plVAO);
		glDrawElements(GL_TRIANGLES, sizeof(sphIndices) / sizeof(unsigned long), GL_UNSIGNED_INT, 0);
	}
	
	// Display objects
	{
		stdShader.use();
		stdShader.setMat4("projection", projection);
		stdShader.setMat4("view", view);
		stdShader.setVec3("viewPos", (glm::vec3)cam.wPos);
		sb_obj.glRender(stdShader);
	}

	// Display custom sky (if available & enabled)
	if (skyboxState == 3)
	{
		skyShader.use();
		skyShader.setMat4("projection", projection);
		skyShader.setMat4("view", glm::mat4(glm::mat3(view))); // remove translation from the view matrix

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyTexture);

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);
		glBindVertexArray(skyboxVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glDepthFunc(GL_LESS);
		glBindVertexArray(0);
	}
	
	// Display markers
	mShader.use();
	mShader.setMat4("view", view);
	mShader.setMat4("projection", projection);

	for (size_t i = 0; i < g_markers.size(); ++i)
	{
		glBindVertexArray(mVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Marker), &g_markers[i], GL_DYNAMIC_DRAW);
		
		// Draw first in GL_LESS not to rewrite depth values of object faces
		mShader.setVec4("fragColor", glm::vec4(0.f, 1.f, 0.f, 1.f));
		glDrawArrays(GL_LINE_STRIP, 0, 2);
		glDrawArrays(GL_LINE_STRIP, 2, 2);
		
		// Draw white line only where green is covered
		glDepthFunc(GL_NOTEQUAL);
		mShader.setVec4("fragColor", glm::vec4(1.f, 1.f, 1.f, 1.f));
		glDrawArrays(GL_LINE_STRIP, 0, 2);
		glDrawArrays(GL_LINE_STRIP, 2, 2);
		
		glDepthFunc(GL_LESS);
	}
}

static void initPlanet()
{
	glGenVertexArrays(1, &plVAO);
	glGenBuffers(1, &plVBO);
	glGenBuffers(1, &plEBO);

	glBindVertexArray(plVAO);

	glBindBuffer(GL_ARRAY_BUFFER, plVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphVerts), sphVerts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphIndices), sphIndices, GL_STATIC_DRAW);

	// Enabling vertex position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Enabling UV mapping
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

static void initSkybox()
{
	#include <models/skybox.h>
	
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	
	glBindVertexArray(skyboxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	
	// Enabling vertex position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// UV mapping will be enabled automatically
}

static void initMarkers()
{
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	
	glBindVertexArray(mVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
	glBufferData(GL_ARRAY_BUFFER, g_markers.size() * sizeof(Marker), &g_markers[0], GL_DYNAMIC_DRAW);

	// Enabling vertex position. No need for UV mapping
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);
}

NAMESPACE_END(graphics);
