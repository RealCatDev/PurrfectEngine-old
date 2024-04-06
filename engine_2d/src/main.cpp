#include <PurrfectEngine/window.hpp>
#include <PurrfectEngine/application.hpp>

int main (int argc, char **argv) {
  PurrfectEngine::application *app = new PurrfectEngine::application();
  app->setAssetDir("../assets/");
  if (argc > 1) app->setAssetDir(argv[1]);

  PurrfectEngine::window   *window   = new PurrfectEngine::window("PurrfectEngine", 1920, 1080);


  while (!window->shouldClose()) {
    glfwPollEvents();
  }
  delete window;

  return 0;
}