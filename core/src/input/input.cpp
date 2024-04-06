#include "PurrfectEngine/input/input.hpp"
#include "PurrfectEngine/window.hpp"

namespace PurrfectEngine::Input {

  input::input(::PurrfectEngine::window *win):
    mWindow(win->get()) {
      glfwSetWindowUserPointer(mWindow, this);

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

  void input::addScrollCallback(std::function<void(double, double)> callback) {
    mScrollCallbacks.push_back(callback);
  }

}