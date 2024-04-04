#ifndef   ENGINE_PURRENGINE_CAMERA_HPP_
#define   ENGINE_PURRENGINE_CAMERA_HPP_

#include <PurrfectEngine/input/input.hpp>
#include <PurrfectEngine/transform.hpp>
#include <PurrfectEngine/camera.hpp>

namespace PurrfectEngine {

  class EditorCamera {
  public:
    EditorCamera(Input::input *input):
      mInput(input), mMovementSpeed(1.0f), mIsScrolling(false) {
      mTransform = new purrTransform();
      mCamera = new purrCamera(mTransform);

      mKeyBindings = {
        { Input::keyBinds::KEY_W, [this]() { moveForward(); } },
        { Input::keyBinds::KEY_S, [this]() { moveBackward(); } },
        { Input::keyBinds::KEY_A, [this]() { strafeLeft(); } },
        { Input::keyBinds::KEY_D, [this]() { strafeRight(); } }
      };

      mInput->setMouseButtonCallback([this](int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
          if (action == GLFW_PRESS) {
            mIsScrolling = true;
            mLastMousePos = mInput->getMousePosition();
          } else if (action == GLFW_RELEASE) {
            mIsScrolling = false;
          }
        }
      });

      mInput->setMouseMoveCallback([this](double xPos, double yPos) {
        if (mIsScrolling) {
            glm::vec2 mousePos = glm::vec2(xPos, yPos);
            glm::vec2 delta = (mousePos - mLastMousePos) * 0.01f;

            float pitch = mTransform->getRotation().x - delta.y;
            float yaw = mTransform->getRotation().y - delta.x;
            pitch = glm::clamp(pitch, -89.0f, 89.0f);

            mTransform->setRotation(glm::vec3(pitch, yaw, 0.0f));

            mLastMousePos = mousePos;
        }
    });


      mInput->addScrollCallback([this](double xOffset, double yOffset) { 
        if (mInput->isKeyPressed(Input::keyBinds::KEY_LEFT_ALT) || mInput->isKeyPressed(Input::keyBinds::KEY_RIGHT_ALT)) {
          mMovementSpeed += yOffset * 0.1f;
          if (mMovementSpeed < 0.1f) mMovementSpeed = 0.1f;
        }
      });
    }

    ~EditorCamera() {
      delete mCamera;
      delete mTransform;
    }

    // dt = delta time
    void update(double dt) {
      for (const auto& [key, action] : mKeyBindings) {
        if (mInput->isKeyPressed(key)) {
          action();
        }
      }
    }

    purrCamera *get() const { return mCamera; }
  private:
    Input::input *mInput = nullptr;

    purrTransform *mTransform = nullptr;
    purrCamera    *mCamera = nullptr;

    std::unordered_map<Input::keyBinds, std::function<void()>> mKeyBindings;

    float mMovementSpeed;
    bool mIsScrolling;
    glm::vec2 mLastMousePos;

    void moveForward() {
      mTransform->getPosition() += mTransform->getForward() * glm::vec3(0.0f, 0.0f, mMovementSpeed);
    }

    void moveBackward() {
      mTransform->getPosition() += mTransform->getForward() * glm::vec3(0.0f, 0.0f, -mMovementSpeed);
    }

    void strafeLeft() {
      mTransform->getPosition() += mTransform->getRight() * glm::vec3(-mMovementSpeed, 0.0f, 0.0f);
    }

    void strafeRight() {
      mTransform->getPosition() += mTransform->getRight() * glm::vec3(mMovementSpeed, 0.0f, 0.0f);
    }
  };

}

#endif // ENGINE_PURRENGINE_CAMERA_HPP_
