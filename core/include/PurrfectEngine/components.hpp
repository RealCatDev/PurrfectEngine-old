#ifndef PURRENGINE_COMPONENTS_HPP_
#define PURRENGINE_COMPONENTS_HPP_

#include "PurrfectEngine/mesh.hpp"
#include "PurrfectEngine/transform.hpp"

namespace PurrfectEngine {

  class purrComponent {
  public:
    purrComponent(const char *name)
      : mName(name)
    {}

    ~purrComponent() {
      free((void*)mName);
    }

    const char *getName() const { return mName; }
  private:
    const char *mName;
  };

  class transformComponent : public purrComponent {
  public:
    transformComponent(purrTransform *transform)
      : purrComponent("Transform"), mTransform(transform)
    {}

    purrTransform *get() const { return mTransform; }
  private:
    purrTransform *mTransform = nullptr;
  };

  class meshComponent : public purrComponent {
  public:
    meshComponent(vkMesh *mesh)
      : purrComponent("Mesh"), mMesh(mesh)
    {}

    ~meshComponent() {
      if (mMesh) delete mMesh;
    }

    vkMesh *get() const { return mMesh; }
  private:
    vkMesh *mMesh = nullptr;
  };

}

#endif