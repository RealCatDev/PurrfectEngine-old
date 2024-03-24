#ifndef PURRENGINE_LOADERS_HPP_
#define PURRENGINE_LOADERS_HPP_

#include "PurrfectEngine/core.hpp"
#include "PurrfectEngine/renderer.hpp"
#include "PurrfectEngine/mesh.hpp"

namespace PurrfectEngine {

  class modelLoader {
  public:
    modelLoader(vkRenderer *renderer);
    ~modelLoader();
  public:
    static vkMesh *load(const char *filename, vkCommandPool *cmdPool);
    static void save(const char *filename);
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    inline static modelLoader *sInstance = nullptr;
  };

}

#endif