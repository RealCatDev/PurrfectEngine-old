#include "PurrfectEngine/input/input.hpp"
#include "PurrfectEngine/window.hpp"

namespace PurrfectEngine::Input {

  input::input(::PurrfectEngine::window *win):
    mWindow(win->get()) {
      glfwSetWindowUserPointer(mWindow, this);

      glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto input = static_cast<Input::input*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS) {
          if (input->mKeyBindings.find(static_cast<keyBinds>(key)) != input->mKeyBindings.end()) {
            input->mKeyBindings[static_cast<keyBinds>(key)]();
          }
        }
      });

      glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xOffset, double yOffset) {
        auto input = static_cast<Input::input*>(glfwGetWindowUserPointer(window));
        for (const auto& callback : input->mScrollCallbacks) {
          callback(xOffset, yOffset);
        }
      });
    }

  input::~input() {

  }

  bool input::isKeyPressed(keyBinds key) {
    return glfwGetKey(mWindow, static_cast<int>(key)) == GLFW_PRESS;
  }

  bool input::isKeyReleased(keyBinds key) {
    return glfwGetKey(mWindow, static_cast<int>(key)) == GLFW_RELEASE;
  }

  bool input::isMouseButtonDown(mouseButton button) {
    return glfwGetMouseButton(mWindow, static_cast<int>(button)) == GLFW_PRESS;
  }

  bool input::isMouseButtonReleased(mouseButton button) {
    return glfwGetMouseButton(mWindow, static_cast<int>(button)) == GLFW_RELEASE;
  }

  glm::vec2 input::getMousePosition() {
    double xPos, yPos;
    glfwGetCursorPos(mWindow, &xPos, &yPos);
    return glm::vec2(xPos, yPos);
  }

  glm::vec2 input::getMouseDelta() {
    static double lastX = 0, lastY = 0;
    double xPos, yPos;
    glfwGetCursorPos(mWindow, &xPos, &yPos);
    double deltaX = xPos - lastX;
    double deltaY = yPos - lastY;
    lastX = xPos;
    lastY = yPos;
    return glm::vec2(deltaX, deltaY);
  }

  void input::setKeyBinding(keyBinds key, std::function<void()> action) {
    mKeyBindings[key] = action;
  }

  void input::addScrollCallback(std::function<void(double, double)> callback) {
    mScrollCallbacks.push_back(callback);
  }

  void input::setMouseButtonCallback(std::function<void(int, int, int)> callback) {
    mMouseButtonCallback = callback;
    glfwSetMouseButtonCallback(mWindow, glfwMouseButtonCallback);
}

void input::setMouseMoveCallback(std::function<void(double, double)> callback) {
    mMouseMoveCallback = callback;
    glfwSetCursorPosCallback(mWindow, glfwMouseMoveCallback);
}

void input::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    input* inputInstance = reinterpret_cast<input*>(glfwGetWindowUserPointer(window));
    if (inputInstance && inputInstance->mMouseButtonCallback) {
        inputInstance->mMouseButtonCallback(button, action, mods);
    }
}

void input::glfwMouseMoveCallback(GLFWwindow* window, double xPos, double yPos) {
    input* inputInstance = reinterpret_cast<input*>(glfwGetWindowUserPointer(window));
    if (inputInstance && inputInstance->mMouseMoveCallback) {
        inputInstance->mMouseMoveCallback(xPos, yPos);
    }
}

}