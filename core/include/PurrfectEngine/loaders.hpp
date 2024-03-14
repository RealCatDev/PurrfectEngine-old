#ifndef PURRENGINE_LOADERS_HPP_
#define PURRENGINE_LOADERS_HPP_

#include "PurrfectEngine/core.hpp"
#include "PurrfectEngine/renderer.hpp"
#include "PurrfectEngine/model.hpp"

namespace PurrfectEngine {

  class modelLoader {
  public:
    modelLoader(vkRenderer *renderer);
    ~modelLoader();
  public:
    static bool load(const char *filename, vkCommandPool *cmdPool, vkModel **model);
    static void save(const char *filename);
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    inline static modelLoader *sInstance = nullptr;
  };

}

#endif