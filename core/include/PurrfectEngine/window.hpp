#ifndef PURRENGINE_WINDOW_HPP_
#define PURRENGINE_WINDOW_HPP_

#include "PurrfectEngine/core.hpp"

#include <functional>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include "./inputsrc/input.hpp"

namespace PurrfectEngine {

  class cursor {
  public:
    cursor();
    cursor(const char *filename);
    cursor(const cursor &) = delete;
    cursor(const cursor &&) = delete;
    ~cursor();

    static cursor *standard(int shape) {
      auto cur = new cursor();
      cur->mCursor = glfwCreateStandardCursor(shape);
      return cur;
    }
  
    void load(const char *filename);
    void set(GLFWwindow *window);
  private:
    GLFWcursor *mCursor;
  };

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
    window(const window &) = delete;
    window(const window &&) = delete;
    ~window();

    windowState &getState() { return mState; }

    void setFullscreen(bool fullscreen);
    void setWidth(int width);
    void setHeight(int height);
    void setX(int x);
    void setY(int y);
    void setCursor(cursor *cur);

    void getSize(int *x, int *y) { glfwGetWindowSize(mWindow, x, y); }

    std::vector<const char *> getVkExtensions() {
      uint32_t glfwExtensionCount = 0;
      const char** glfwExtensions;
      glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
      
      return std::vector<const char *>(glfwExtensions, glfwExtensions + glfwExtensionCount);
    }

    bool shouldClose() const { return glfwWindowShouldClose(mWindow); }

    GLFWwindow *get() const { return mWindow; }
  private:
    GLFWwindow *mWindow = NULL;
    purrInput *mInput;
    windowState mState{};
  };

}

#endif