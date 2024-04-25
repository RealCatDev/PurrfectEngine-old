#include "PurrfectEngine/application.hpp"

#include "PurrfectEngine/core.hpp"
#include <filesystem>
namespace PurrfectEngine {

  application *application::getInstance() { PURR_ASSERT(sInstance); return sInstance; }

  application::application():
    mAssetDir(nullptr)
  { PURR_ASSERT(!sInstance); sInstance = this;}

  application::~application() {

  }

  void application::setAssetDir(const char *dir) {
    if (mAssetDir) free(mAssetDir);
    mAssetDir = (char*)malloc(strlen(dir));
    strcpy(mAssetDir, dir);
  }

  void application::setProjDir(const char *dir) {
    if (mProjDir) free(mProjDir);
    mProjDir = (char*)malloc(strlen(dir));
    strcpy(mProjDir, dir);
  }

  const char *application::getAssetDir() const { return mAssetDir; }
  const char *application::getProjDir() const { return mProjDir; }
  
}