#ifndef PURRENGINE_CAMERA_HPP_
#define PURRENGINE_CAMERA_HPP_

#include "PurrfectEngine/transform.hpp"

namespace PurrfectEngine {

  struct CameraUBO {
    glm::mat4 proj;
    glm::mat4 view;
    glm::vec4 pos;
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
      float aspectRatio = size.x / (float) size.y;
      float s = aspectRatio;
      float g = s / glm::tan(glm::radians(mFOV) / 2.f);
      float fovY = 2.f * glm::atan(1.f / g);

      mProj = glm::perspective(fovY, aspectRatio, mNearPlane, mFarPlane);
      mProj[1][1] *= -1;
    }

    glm::mat4 getProjection() const { return mProj; }
    glm::mat4 getView()             { return mView = glm::lookAt(mTransform->getPosition(), mTransform->getPosition() + mTransform->getForward(), mTransform->getUp()); }
    glm::vec3 getPosition()   const { return mTransform->getPosition(); }
  private:
    float mFOV = 90.0f, mNearPlane = 0.01f, mFarPlane = 100.0f;
    glm::mat4 mProj, mView;
    purrTransform *mTransform = nullptr;
  };

}

#endif