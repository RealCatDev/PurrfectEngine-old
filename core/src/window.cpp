#include "PurrfectEngine/window.hpp"

#include <stb_image.h>

namespace PurrfectEngine {

  cursor::cursor():
    mCursor(NULL) 
  {}

  cursor::cursor(const char *filename):
    mCursor(NULL)
  { load(filename); }

  cursor::~cursor() {
    glfwDestroyCursor(mCursor);
  }

  void cursor::load(const char *filename) {
    int width = 0, height = 0, channels = 0;
    stbi_uc *pixels = stbi_load(filename, &width, &height, &channels, STBI_grey_alpha);
    if (!pixels) {
      throw std::runtime_error("Failed to load cursor: \"" + std::string(filename) + "\" not found!");
    }

    GLFWimage image{};
    image.width = width;
    image.height = height;
    image.pixels = pixels;

    GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
    if (!cursor) {
      throw std::runtime_error("Failed to create cursor from image: " + std::string(filename));
    }

    stbi_image_free(pixels);
  }

  void cursor::set(GLFWwindow *window) {
    glfwSetCursor(window, mCursor);
  }

  window::window(const char *title, int width, int height):
    mWindow(NULL)
  {
    if(!glfwInit()) throw std::runtime_error("Failed to initialize GLFW!");

    if(!glfwVulkanSupported()) throw std::runtime_error("Failed to initialize GLFW: Vulkan is not supported!");

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

  void window::setCursor(cursor *cur) {
    cur->set(mWindow);
  }

}