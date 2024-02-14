#ifndef PURRENGINE_CAMERA_HPP_
#define PURRENGINE_CAMERA_HPP_

#include "PurrfectEngine/transform.hpp"

namespace PurrfectEngine {

  struct CameraUBO {
    glm::mat4 proj;
    glm::mat4 view;
  };

  class purrCamera {
  public:
    purrCamera(purrTransform *trans):
      mTransform(trans)
    {}

    ~purrCamera() {
      delete mTransform;
    }

    void calculate(glm::ivec2 size) {
      mProj = glm::perspective(glm::radians(mFOV), size.x / (float) size.y, mNearPlane, mFarPlane);
      mProj[1][1] *= -1;
      mView = glm::lookAt(mTransform->getPosition(), mTransform->getPosition() + mTransform->getForward(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::mat4 getProjection() const { return mProj; }
    glm::mat4 getView()       const { return mView; }
  private:
    float mFOV = 45.0f, mNearPlane = 0.1f, mFarPlane = 100.0f;
    glm::mat4 mProj, mView;
    purrTransform *mTransform = nullptr;
  };

}

#endif