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

      mInput->addScrollCallback([this](double xOffset, double yOffset) {
        if (mInput->isKeyPressed(Input::keyBinds::KEY_LEFT_ALT)) { mMovementSpeed += yOffset; mMovementSpeed = glm::clamp(mMovementSpeed, 0.1f, 10.0f); }
      });
    }

    ~EditorCamera() {
      delete mCamera;
      delete mTransform;
    }

    // dt = delta time
    void update(double dt) {
      glm::vec2 delta;
      {
        auto mousePos = mInput->getMousePosition();
        delta = (mousePos - mMousePosition) * mMouseSensivity;
        mMousePosition = mousePos;
      }

      float z = mInput->isKeyPressed(Input::keyBinds::KEY_W) - mInput->isKeyPressed(Input::keyBinds::KEY_S);
      float x = mInput->isKeyPressed(Input::keyBinds::KEY_D) - mInput->isKeyPressed(Input::keyBinds::KEY_A);
      glm::vec3 pos = glm::vec3(0.0f);
      pos += mTransform->getForward() * z;
      pos += mTransform->getRight()   * x;
      pos *= mMovementSpeed * dt;
      mTransform->getPosition() += pos;

      if (mInput->isMouseButtonDown(Input::mouseButton::MOUSE_RIGHT)) {
        mYaw += delta.x;
        mPitch += delta.y;
        mPitch = glm::clamp(mPitch, -89.0f, 89.0f);
        mTransform->setRotation(glm::vec3(-mPitch, -mYaw, 0.0f));
      }
    }

    purrCamera *get() const { return mCamera; }
  private:
    Input::input *mInput = nullptr;

    purrTransform *mTransform = nullptr;
    purrCamera    *mCamera = nullptr;

    float mMovementSpeed, mMouseSensivity = 0.02f;
    bool mIsScrolling;
    glm::vec2 mMousePosition;
    float mYaw = 0.0f, mPitch = 0.0f;
  };

}

#endif // ENGINE_PURRENGINE_CAMERA_HPP_
