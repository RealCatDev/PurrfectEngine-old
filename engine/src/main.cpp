#include <PurrfectEngine/scene.hpp>

#include "renderer.hpp"

#include <PurrfectEngine/window.hpp>
#include <PurrfectEngine/application.hpp>

int main (int argc, char **argv) {
  PurrfectEngine::application *app = new PurrfectEngine::application();
  app->setAssetDir("../assets/");
  if (argc > 1) app->setAssetDir(argv[2]);

  PurrfectEngine::window   *window   = new PurrfectEngine::window("PurrfectEngine", 1920, 1080);
  PurrfectEngine::renderer *renderer = new PurrfectEngine::renderer(window);
  renderer->initialize();

  PurrfectEngine::purrScene *scene = new PurrfectEngine::purrScene();
  {
    auto mesh = renderer->createMesh(PurrfectEngine::Asset("models/teapot.obj"));

    auto obj = new PurrfectEngine::purrObject("meow");
    obj->addComponent(new PurrfectEngine::meshComponent(mesh));
    scene->addObject(obj);
  }
  renderer->setScene(scene);

  while (!window->shouldClose()) {
    glfwPollEvents();
    renderer->render();
  }

  delete scene;

  delete renderer;
  delete window;

  return 0;
}