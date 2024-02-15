#include "renderer.hpp"

#include <PurrfectEngine/window.hpp>

int main (int argc, char **argv) {
  PurrfectEngine::window   *window   = new PurrfectEngine::window("PurrfectEngine", 1920, 1080);
  PurrfectEngine::renderer *renderer = new PurrfectEngine::renderer(window);
  renderer->initialize();

  while (!window->shouldClose()) {
    glfwPollEvents();
    renderer->render();
  }
  delete renderer;
  delete window;

  return 0;
}