#include "PurrfectEngine/input/input.hpp"
#include "PurrfectEngine/window.hpp"

namespace PurrfectEngine::Input {

  input::input(::PurrfectEngine::window *win):
    mWindow(win->get()) {}

  input::~input() {

  }

  bool input::isKeyPressed(keyBinds key) {
    return glfwGetKey(mWindow, static_cast<int>(key)) == GLFW_PRESS && !mKeyState[key];
  }

  bool input::isKeyReleased(keyBinds key) {
    return glfwGetKey(mWindow, static_cast<int>(key)) == GLFW_RELEASE && mKeyState[key];
  }

  void input::updateKeyState() {
    for (auto& [key, value] : mKeyState) value = glfwGetKey(mWindow, static_cast<int>(key)) == GLFW_PRESS;
  }

  bool input::isMouseButtonDown(mouseButton button) {
    return glfwGetMouseButton(mWindow, static_cast<int>(button)) == GLFW_PRESS && !mMouseButtonState[button];
  }

  bool input::isMouseButtonReleased(mouseButton button) {
    return glfwGetMouseButton(mWindow, static_cast<int>(button)) == GLFW_RELEASE && mMouseButtonState[button];
  }

  void input::updateMouseButtonState() {
    for (auto& [button, value] : mMouseButtonState) value = glfwGetMouseButton(mWindow, static_cast<int>(button)) == GLFW_PRESS;
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

}