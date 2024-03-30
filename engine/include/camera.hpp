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

    }

    void update() {
      
    }
  private:
    Input::input *mInput = nullptr;

    purrTransform *mTransform = nullptr;
    purrCamera    *mCamera = nullptr;
  };

}

#endif // ENGINE_PURRENGINE_CAMERA_HPP_