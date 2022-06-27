// 27 june 2022
// SBView v0.1.0-alpha

#ifndef CALLBACKS_HPP
#define CALLBACKS_HPP

#include <common.hpp>
#include <GLFW/glfw3.h>

NAMESPACE_BEGIN(input)

enum cbEnum {
	KEY_MODS,
	MOUSE_STATE
};

int getVal(cbEnum query);

void keyCallbackEvent(GLFWwindow*, int, int, int, int);
void framebufResizeCallbackEvent(GLFWwindow*, int, int);
void mouseButtonCallbackEvent(GLFWwindow*, int, int, int);
void cursorPosCallbackEvent(GLFWwindow*, double, double);
void scrollCallbackEvent(GLFWwindow*, double, double);

int getKeyboardMods();
int getMouseState();

NAMESPACE_END(input)

#endif