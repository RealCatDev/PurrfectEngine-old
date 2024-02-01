#include "PurrfectEngine/window.hpp"

namespace PurrfectEngine {

  window::window(const char *title, int width, int height)
  {
    PURR_ASSERT(glfwInit());

    PURR_ASSERT(glfwVulkanSupported());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(width, height, title, NULL, NULL);
    PURR_ASSERT(mWindow);

    mState.Width = width;
    mState.Height = height;
    mState.Fullscreen = false;

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    int x = 0, y = 0;
    glfwGetMonitorWorkarea(monitor, NULL, NULL, &x, &y);
    x /= 2;
    x -= width / 2;
    y /= 2;
    y -= height / 2;
    setX(x);
    setY(y);
    free(monitor);

    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow *win, int w, int h) {
      window *Window = (window*)glfwGetWindowUserPointer(win);
      auto state = Window->getState();
      state.Width = w;
      state.Height = h;
    });

    glfwSetWindowPosCallback(mWindow, [](GLFWwindow *win, int x, int y) {
      window *Window = (window*)glfwGetWindowUserPointer(win);
      auto state = Window->getState();
      state.xPos = x;
      state.yPos = y;
    });
  }

  window::~window() {
    glfwDestroyWindow(mWindow);
    glfwTerminate();
  }

  void window::setFullscreen(bool fs) {
    if (mState.Fullscreen == fs) return; // Same state = don't do nothing
    if (fs) {
      glfwGetWindowSize(mWindow, &mState.Width, &mState.Height);
      glfwGetWindowPos(mWindow, &mState.xPos, &mState.yPos);
    }
    GLFWmonitor *monitor = fs ? glfwGetPrimaryMonitor() : NULL;
    int x = fs ? 0 : mState.xPos, y = fs ? 0 : mState.yPos, w = mState.Width, h = mState.Height;
    if (fs) glfwGetMonitorWorkarea(monitor, NULL, NULL, &w, &h);
    glfwSetWindowMonitor(mWindow, monitor, x, y, w, h, GLFW_DONT_CARE);
  }

  void window::setWidth(int w) {
    glfwSetWindowSize(mWindow, mState.Width = w, mState.Height);
  }

  void window::setHeight(int h) {
    glfwSetWindowSize(mWindow, mState.Width, mState.Height = h);
  }

  void window::setX(int x) {
    glfwSetWindowPos(mWindow, mState.xPos = x, mState.yPos);
  }

  void window::setY(int y) {
    glfwSetWindowPos(mWindow, mState.xPos, mState.yPos = y);
  }

}