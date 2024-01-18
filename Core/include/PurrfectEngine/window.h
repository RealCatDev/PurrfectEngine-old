#ifndef PURRENGINE_WINDOW_H_
#define PURRENGINE_WINDOW_H_

#include "includes.h"
#include <GLFW/glfw3.h>

namespace PurrEngine {

	struct WindowOptions {
		const char* Title;
		int Width, Height;
		int xPos, yPos;
		bool Resizable = true;

		WindowOptions(
			const char* title,
			int width, int height,
			int xpos, int ypos
		) : Title(title), Width(width), Height(height), xPos(xpos), yPos(ypos) {}
	};

	class Window {
	public:
		Window() = default;
		Window(const Window&) = delete;  // Copy
		Window(const Window&&) = delete; // Copy
		Window(const WindowOptions &options); // [x/y] = -1 sets it to center of the screen

		~Window();

		void Update();

		GLFWwindow* GetHandle() const { return m_Window; }
	public:
		static bool VulkanSupported() { return glfwVulkanSupported(); }
	private:
		GLFWwindow* m_Window = nullptr;
	};

}

#endif