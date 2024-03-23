#ifndef PURRENGINE_INPUT_INPUT_HPP_
#define PURRENGINE_INPUT_INPUT_HPP_

#include "PurrfectEngine/input/inputDefines.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <unordered_map>

namespace PurrfectEngine {
    class window;
namespace Input {

    class input {
    public:
        input(::PurrfectEngine::window *win);
        ~input();

        bool isKeyPressed(keyBinds key);
        bool isKeyReleased(keyBinds key);
        void updateKeyState();

        bool isMouseButtonDown(mouseButton button);
        bool isMouseButtonReleased(mouseButton button);
        void updateMouseButtonState();
        
        glm::vec2 getMousePosition();
        glm::vec2 getMouseDelta();
    private:
        GLFWwindow* mWindow;
        std::unordered_map<keyBinds, bool> mKeyState;
        std::unordered_map<mouseButton, bool> mMouseButtonState;
    };

}
}

#endif