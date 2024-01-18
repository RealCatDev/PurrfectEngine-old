#include "PurrfectEngine/window.h"

namespace PurrEngine {

	Window::Window(const WindowOptions& options)
	{
		if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW!"); // TODO(CatDev): Maybe better errors?

		glfwInitHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwInitHint(GLFW_RESIZABLE, GLFW_TRUE); // TODO(CatDev): Get options
		
		m_Window = glfwCreateWindow(options.Width, options.Height, options.Title, NULL, NULL);
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		int monitorWidth, monitorHeight;
		glfwGetMonitorWorkarea(monitor, NULL, NULL, &monitorWidth, &monitorHeight);
		int xPos = options.xPos, yPos = options.yPos;
		if (options.xPos < 0) xPos = (monitorWidth / 2) - (options.Width / 2);
		if (options.yPos < 0) yPos = (monitorHeight / 2) - (options.Height / 2);
		glfwSetWindowPos(m_Window, xPos, yPos);
	}

	Window::~Window() {
		glfwDestroyWindow(m_Window);
	}

	void Window::Update() {
		glfwPollEvents();
	}

}