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
    }

    ~EditorCamera() {
      delete mCamera;
      delete mTransform;
    }

    // dt = delta time
    void update(double dt) {
      int x = ((int)mInput->isKeyPressed(Input::keyBinds::KEY_D)) - ((int)mInput->isKeyPressed(Input::keyBinds::KEY_A));
      int z = ((int)mInput->isKeyPressed(Input::keyBinds::KEY_W)) - ((int)mInput->isKeyPressed(Input::keyBinds::KEY_S));
      glm::vec3 finalPos = glm::vec3(0.0f);
      finalPos += mTransform->getForward() * glm::vec3(0.0f, 0.0f, z);
      finalPos += -mTransform->getRight() * glm::vec3(x, 0.0f, 0.0f);
      finalPos *= (float)dt;
      mTransform->getPosition() += finalPos;
    }

    purrCamera *get() const { return mCamera; }
  private:
    Input::input *mInput = nullptr;

    purrTransform *mTransform = nullptr;
    purrCamera    *mCamera = nullptr;
  };

}

#endif // ENGINE_PURRENGINE_CAMERA_HPP_