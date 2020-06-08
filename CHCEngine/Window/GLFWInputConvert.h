#pragma once
#include "InputTable.h"
namespace CHCEngine
{
	namespace Window
	{
		Key convertToKey(int glfwKey);
		MouseButton convertToMouseButton(int glfwButton);
		Action convertToAction(int glfwAction);
	}
}