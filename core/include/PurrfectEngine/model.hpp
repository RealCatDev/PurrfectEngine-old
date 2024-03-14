#ifndef PURRENGINE_MODEL_HPP_
#define PURRENGINE_MODEL_HPP_

#include "PurrfectEngine/core.hpp"
#include "PurrfectEngine/mesh.hpp"

namespace PurrfectEngine {

  class vkModel {
  public:
    vkModel(vkRenderer *renderer):
      mRenderer(renderer)
    {}

    ~vkModel() {
      
    }

    void setMeshes(std::vector<vkMesh*> meshes) { mMeshes = meshes; }
  private:
    std::vector<vkMesh*> mMeshes;
  private:
    vkRenderer *mRenderer = nullptr;
  };

}

#endif