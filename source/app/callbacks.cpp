// 20 june 2022
// SB Engine view program v0.4.0-alpha

#include <app/callbacks.hpp>
#include <app/graphics.hpp>
#include <runtime/print.hpp>
#include <runtime/sbObject.hpp>
#include <stb_image.h>
#include <iostream>

/* Global state variables defined in main.cpp */
extern unsigned long g_markerIndex1, g_markerIndex2;

extern double g_plMass; // kg
extern const double g_plRadius; // metres

extern float g_hardness; // hardness of struts and collisions
extern float g_visc; // viscosity of struts and collisions (0...1)

extern int g_tPrec; // time divisor for spheres & other objects
extern double g_tMul; // time multiplier (0: time stopped)

extern double g_forceMul; // force multiplier

extern Camera* getCurrentCamera();
extern sbObject* getObject();

NAMESPACE_BEGIN(input)

static int kMods = 0; // shift control alt etc
static int mState = 0; // LMB RMB

static unsigned long doStep(unsigned long index);

int getVal(cbEnum query) {
	if (query == KEY_MODS)
		return kMods;
	if (query == MOUSE_STATE)
		return mState;
	return 0;
}

void keyCallbackEvent(GLFWwindow* wnd, int key, int scancode, int action, int mods)
{
	// Copy modifiers states to kMods to use them outside of this function
	kMods = mods;

	if (action != GLFW_RELEASE)
	{
		if (key == GLFW_KEY_ESCAPE && glfwGetInputMode(wnd, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
			glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		else if (key == GLFW_KEY_G)
		{
			if (mods & GLFW_MOD_ALT)
				g_visc -= 0.05f;
			else g_visc += 0.05f;
			
			if (g_visc < 0.f)
				g_visc = 0.f;
			else if (g_visc > 1.f)
				g_visc = 1.f;
		}
		else if (key == GLFW_KEY_U)
		{
			if (mods & GLFW_MOD_ALT)
				g_forceMul /= 1.1;
			else g_forceMul *= 1.1;
		}
		else if (key == GLFW_KEY_H)
		{
			if (mods & GLFW_MOD_ALT)
				g_hardness /= 1.1;
			else g_hardness *= 1.1;
		}
		else if (key == GLFW_KEY_M)
		{
			if (mods & GLFW_MOD_ALT)
				g_plMass /= 1.1;
			else g_plMass *= 1.1;
		}
		// On high time speeds (> 0.01) model crashed and falls apart
		/* else if (key == GLFW_KEY_N)
		 	g_tMul = -g_tMul; */
		else if (key == GLFW_KEY_B) {
			std::cout << "--values:\n\tg_hardness == " << g_hardness <<
				"\n\tg_visc == " << g_visc <<
				"\n\tg_tPrec == " << g_tPrec <<
				"\n\tg_tMul == " << g_tMul <<
				"\n\tg_forceMul == " << g_forceMul <<
				"\n\tg_plMass == " << g_plMass <<
				"\n\tModel coords == "; print(getObject()->getPosition());
				std::cout << "\tModel velocity == "; print(getObject()->getVelocity());
		}
		else if (key == GLFW_KEY_Y && action == GLFW_PRESS)
		{
			graphics::toggleSkybox();
		}
		else if (key == GLFW_KEY_F)
		{
			g_markerIndex1 = doStep(g_markerIndex1);
			std::cout << "g_markerIndex1 is now " << g_markerIndex1 << '\n';
		}
		else if (key == GLFW_KEY_V)
		{
			g_markerIndex2 = doStep(g_markerIndex2);
			std::cout << "g_markerIndex2 is now " << g_markerIndex2 << '\n';
		}
	}
}

void framebufResizeCallbackEvent(GLFWwindow* wnd, int width, int height)
{
	graphics::resizeCallbackEvent(wnd, width, height);
}

void mouseButtonCallbackEvent(GLFWwindow* wnd, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action != GLFW_RELEASE)
		{
			input::mState |= (1 << GLFW_MOUSE_BUTTON_LEFT);
			if (glfwGetInputMode(wnd, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
			{
				glfwSetInputMode(wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				getCurrentCamera()->fInput |= CAM_MOUSE_ENTRY;
			}
		}
		else input::mState &= ~(1 << GLFW_MOUSE_BUTTON_LEFT);
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action != GLFW_RELEASE)
			input::mState |= (1 << GLFW_MOUSE_BUTTON_RIGHT);
		else input::mState &= ~(1 << GLFW_MOUSE_BUTTON_RIGHT);
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		if (action != GLFW_RELEASE)
		{
			input::mState |= (1 << GLFW_MOUSE_BUTTON_MIDDLE);
			getCurrentCamera()->renderDist = 500.f;
			getCurrentCamera()->fInput |= CAM_VIEW_CHANGE;
		}
		else input::mState &= ~(1 << GLFW_MOUSE_BUTTON_MIDDLE);
	}
}

void cursorPosCallbackEvent(GLFWwindow* wnd, double xpos, double ypos)
{
	if (glfwGetInputMode(wnd, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
		getCurrentCamera()->applyRotation((float)xpos, (float)ypos);
}

void scrollCallbackEvent(GLFWwindow* wnd, double, double yoffset)
{
	getCurrentCamera()->glfwScrollCallback(yoffset, kMods);
}

static unsigned long doStep(unsigned long index)
{
	if (kMods & GLFW_MOD_ALT) --index;
	else ++index;

	index %= getObject()->getNumMassPoints();
	return index;
}

NAMESPACE_END(input)