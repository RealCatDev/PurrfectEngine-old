#ifndef   ENGINE_PURRENGINE_CAMERA_HPP_
#define   ENGINE_PURRENGINE_CAMERA_HPP_

#include <PurrfectEngine/input/input.hpp>
#include <PurrfectEngine/transform.hpp>
#include <PurrfectEngine/camera.hpp>

namespace PurrfectEngine {

  class EditorCamera {
  public:
    EditorCamera(Input::input *input):
      mInput(input) {
      mTransform = new purrTransform();
      mCamera = new purrCamera(mTransform);

      mKeyBindings = {
        { Input::keyBinds::KEY_W, [this]() { moveForward(); } },
        { Input::keyBinds::KEY_S, [this]() { moveBackward(); } },
        { Input::keyBinds::KEY_A, [this]() { strafeLeft(); } },
        { Input::keyBinds::KEY_D, [this]() { strafeRight(); } }
      };

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
      printf(mMovementSpeed);
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

    int mMovementSpeed;

    void moveForward() {
      mTransform->getPosition() += mTransform->getForward() * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    void moveBackward() {
      mTransform->getPosition() += mTransform->getForward() * glm::vec3(0.0f, 0.0f, 1.0f);
    }

    void strafeLeft() {
      mTransform->getPosition() += mTransform->getRight() * glm::vec3(-1.0f, 0.0f, 0.0f);
    }

    void strafeRight() {
      mTransform->getPosition() += mTransform->getRight() * glm::vec3(1.0f, 0.0f, 0.0f);
    }
  };

}

#endif // ENGINE_PURRENGINE_CAMERA_HPP_