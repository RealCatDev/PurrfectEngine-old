#include <PurrfectEngine/window.hpp>

int main (int argc, char **argv) {
  PurrfectEngine::window *window = new PurrfectEngine::window("PurrfectEngine", 1920, 1080);

  while (!window->shouldClose()) {
    glfwPollEvents();
  }
  delete window;

  return 0;
}