#ifndef PURRENGINE_INPUT_INPUT_HPP_
#define PURRENGINE_INPUT_INPUT_HPP_

#include "PurrfectEngine/core.hpp"
#include "PurrfectEngine/input/inputDefines.hpp"

#include "functional"
#include "unordered_map"
#include "vector"

#include <GLFW/glfw3.h>

namespace PurrfectEngine {
  class window;
namespace Input {

  class input {
  public:
    input(window *win);
    ~input();

    bool isKeyPressed(keyBinds key);
    bool isKeyReleased(keyBinds key);

    bool isMouseButtonDown(mouseButton button);
    bool isMouseButtonReleased(mouseButton button);

    glm::vec2 getMousePosition();

    void addScrollCallback(std::function<void(double, double)> callback);
  private:
    GLFWwindow* mWindow;
    std::vector<std::function<void(double, double)>> mScrollCallbacks;
  };

}
}

#endif