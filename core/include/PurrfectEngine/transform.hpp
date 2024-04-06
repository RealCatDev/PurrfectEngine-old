#ifndef PURRENGINE_TRANSFORM_HPP_
#define PURRENGINE_TRANSFORM_HPP_

#include "PurrfectEngine/core.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <algorithm>

namespace PurrfectEngine {

  class purrTransform {
  public:
    purrTransform(glm::vec3 position = glm::vec3(0.0f), glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3 scale = glm::vec3(1.0f)):
      mPos(position), mRot(rotation), mScl(scale)
    {}

    ~purrTransform() {}

    const glm::mat4 getTransform() {
      glm::mat4 transform = glm::translate(glm::mat4(1.0f), mPos);
      if (mRot != glm::identity<glm::quat>()) transform *= glm::mat4_cast(mRot);
      return glm::scale(transform, mScl);
    }
    
    glm::vec3 &getPosition() { return mPos; }
    glm::quat &getRotation() { return mRot; }
    glm::vec3 &getScale() { return mScl; }

    void setPosition(glm::vec3 v) { mPos = v; }
    void setRotation(glm::quat q) { mRot = q; }
    void setScale(glm::vec3 v) { mScl = v; }

    void setYawPitch(float yaw, float pitch) {
      const auto yawRotation   = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
      const auto pitchRotation = glm::angleAxis(pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
      mRot = yawRotation * pitchRotation;
    }

    const glm::vec3 getForward() const {
      return glm::rotate(mRot, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    const glm::vec3 getRight() const {
      return glm::rotate(mRot, glm::vec3(-1.0f, 0.0f, 0.0f));
    }

    const glm::vec3 getUp() const {
      return glm::rotate(mRot, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    const glm::mat3 getNormal() {
      return glm::transpose(glm::inverse(glm::mat3(getTransform())));
    }

  private:
    glm::vec3 mPos, mScl;
    glm::quat mRot;
  };

}

#endif