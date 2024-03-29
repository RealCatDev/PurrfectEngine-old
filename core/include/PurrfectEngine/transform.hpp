#ifndef PURRENGINE_TRANSFORM_HPP_
#define PURRENGINE_TRANSFORM_HPP_

#include "PurrfectEngine/core.hpp"

#include <glm/gtc/matrix_inverse.hpp>

namespace PurrfectEngine {

  class purrTransform {
  public:
    purrTransform(glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f)):
      mPos(position), mRot(rotation), mScl(scale)
    {}

    ~purrTransform() {

    }

    const glm::mat4 getTransform() { 
      return mTransform = glm::translate(glm::mat4(1.0f), mPos)
                          * glm::rotate(glm::mat4(1.0f), glm::radians(mRot.x), glm::vec3(1.0f, 0.0f, 0.0f))
                          * glm::rotate(glm::mat4(1.0f), glm::radians(mRot.y), glm::vec3(0.0f, 1.0f, 0.0f))
                          * glm::rotate(glm::mat4(1.0f), glm::radians(mRot.z), glm::vec3(0.0f, 0.0f, 1.0f))
                          * glm::scale(glm::mat4(1.0f), mScl);
    }
    
    glm::vec3 getPosition() const { return mPos; }
    glm::vec3 getRotation() const { return mRot; }
    glm::vec3 getScale   () const { return mScl; }

    void setPosition(glm::vec3 v) { mPos = v; }
    void setRotation(glm::vec3 v) { mRot = v; }
    void setScale   (glm::vec3 v) { mScl = v; }

    const glm::vec3 getForward() const {
      return glm::vec3(sin(mRot.y), -tan(mRot.x), cos(mRot.y));
    }

    const glm::vec3 getRight() const {
      glm::vec3 forward = getForward();
      forward = glm::normalize(forward);
      glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
      
      return glm::normalize(glm::cross(worldUp, forward));
    }

    const glm::vec3 getUp() const {
      glm::vec3 forward = getForward();
      forward = glm::normalize(forward);
      glm::vec3 right = getRight();

      return glm::normalize(glm::cross(forward, right));
    }

    const glm::mat3 getNormal() {
      return glm::inverseTranspose(glm::mat3(getTransform()));
    }
  private:
    glm::mat4 mTransform;
    glm::vec3 mPos, mRot, mScl;
  };

}

#endif