#ifndef PURRENGINE_WINDOW_HPP_
#define PURRENGINE_WINDOW_HPP_

#include "PurrfectEngine/core.hpp"

#include <functional>
#include <glfw/glfw3.h>

namespace PurrfectEngine {

  struct windowState {
    bool Fullscreen = false;
    int  Width      = 0;
    int  Height     = 0;
    int  xPos       = 0;
    int  yPos       = 0;
  };

  class window {
  public:
    window(const char *title, int width, int height);
    ~window();

    windowState &getState() { return mState; }

    void setFullscreen(bool fullscreen);
    void setWidth(int width);
    void setHeight(int height);
    void setX(int x);
    void setY(int y);

    bool shouldClose() const { return glfwWindowShouldClose(mWindow); }
  private:
    GLFWwindow *mWindow = NULL;
    windowState mState{};
  };

}

#endif