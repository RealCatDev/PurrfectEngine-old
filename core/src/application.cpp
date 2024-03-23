#include "PurrfectEngine/application.hpp"

#include "PurrfectEngine/core.hpp"

namespace PurrfectEngine {

  application *application::getInstance() { PURR_ASSERT(sInstance); return sInstance; }

  application::application():
    mAssetDir(nullptr)
  { PURR_ASSERT(!sInstance); sInstance = this; }

  application::~application() {

  }

  void application::setAssetDir(const char *dir) {
    if (mAssetDir) free(mAssetDir);
    mAssetDir = (char*)malloc(strlen(dir));
    strcpy(mAssetDir, dir);
  }

  const char *application::getAssetDir() const { return mAssetDir; }
  
}