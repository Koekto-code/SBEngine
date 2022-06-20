// 20 june 2022
// SB Engine view program v0.4.0-alpha

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <common.hpp>
#include <GLFW/glfw3.h>

#include <runtime/camera.hpp>
#include <runtime/sbObject.hpp>

NAMESPACE_BEGIN(graphics)

typedef struct Marker {
	glm::vec3 orig;
	glm::vec3 v;
} Marker;

enum grEnum
{
	WIN_WIDTH,
	WIN_HEIGHT,
	SKYBOX_STATE,
	VIEW_CHANGE
};

int getVal(grEnum query);
void resizeCallbackEvent(GLFWwindow* wnd, int width, int height);
void toggleSkybox();
void applySkyboxShaders();

// Create window, fill GL buffers, load textures & compile shaders
int init(GLFWwindow*& wnd);
void shutdown();
void display(Camera& cam, sbObject& sb_obj);

NAMESPACE_END(graphics)

#endif