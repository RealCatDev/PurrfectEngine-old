#include <PurrfectEngine/scene.hpp>
#include <PurrfectEngine/window.hpp>

int main (int argc, char **argv) {
  PurrfectEngine::window   *window   = new PurrfectEngine::window("PurrfectEngine", 1920, 1080);
  PurrfectEngine::purrInput input;


  while (!window->shouldClose()) {
    glfwPollEvents();

    if (input.isKeyPressed(KeyBinds::KEY_A)) {
      std::cout << "Key A pressed" << std::endl;
    }

    if (input.isKeyReleased(KeyBinds::KEY_A)) {
      std::cout << "Key A released" << std::endl;
    }

    // Update key states
    input.updateKeyState();
  }
  delete window;

  return 0;
}