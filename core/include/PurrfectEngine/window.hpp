#ifndef PURRENGINE_WINDOW_HPP_
#define PURRENGINE_WINDOW_HPP_

#include "PurrfectEngine/core.hpp"
#include "PurrfectEngine/input/input.hpp"

#include <functional>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

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
    int xPos, yPos;
    int Width, Height;
    bool Fullscreen = false;
  };

  class window {
  public:
    window(const char *title, int width, int height);
    window(const window &) = delete;
    window(const window &&) = delete;
    ~window();

    windowState &getState() { return mState; }

    void setFullscreen(bool fullscreen);
    void setSize(glm::ivec2 size);
    void setPos(glm::ivec2 pos);
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
    Input::input *mInput;
    windowState mState{};
  };

}

#endif