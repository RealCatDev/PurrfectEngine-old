#ifndef PURRENGINE_INPUTSRC_INPUT_HPP
#define PURRENGINE_INPUTSRC_INPUT_HPP

#include "inputDefines.hpp"
#include <GLFW/glfw3.h>
#include <PurrfectEngine/window.hpp>
#include <unordered_map>

namespace PurrfectEngine {

    class purrInput {
    public:
        purrInput() {}

        void setWindow(GLFWwindow* newWindow) {
            window = newWindow;
        }

        bool isKeyPressed(KeyBinds key) {
            return glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS && !keyState[key];
        }

        bool isKeyReleased(KeyBinds key) {
            return glfwGetKey(window, static_cast<int>(key)) == GLFW_RELEASE && keyState[key];
        }

        void updateKeyState() {
            for (auto& [key, value] : keyState) {
                value = glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS;
            }
        }

        bool isMouseButtonDown(MouseButton button) {
            return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS && !mouseButtonState[button];
        }

        bool isMouseButtonReleased(MouseButton button) {
            return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_RELEASE && mouseButtonState[button];
        }

        void updateMouseButtonState() {
            for (auto& [button, value] : mouseButtonState) {
                value = glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS;
            }
        }

        glm::vec2 getMousePosition() {
            double xPos, yPos;
            glfwGetCursorPos(window, &xPos, &yPos);
            return glm::vec2(xPos, yPos);
        }

        glm::vec2 getMouseDelta() {
            static double lastX = 0, lastY = 0;
            double xPos, yPos;
            glfwGetCursorPos(window, &xPos, &yPos);
            double deltaX = xPos - lastX;
            double deltaY = yPos - lastY;
            lastX = xPos;
            lastY = yPos;
            return glm::vec2(deltaX, deltaY);
        }

    private:
        GLFWwindow* window;
        std::unordered_map<KeyBinds, bool> keyState;
        std::unordered_map<MouseButton, bool> mouseButtonState;
    };
}

#endif
