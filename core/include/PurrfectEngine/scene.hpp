#ifndef PURRENGINE_SCENE_HPP_
#define PURRENGINE_SCENE_HPP_

#include "PurrfectEngine/object.hpp"

namespace PurrfectEngine {

  class purrScene {
  public:
    purrScene()
    {}

    ~purrScene() {
      for (auto obj : mObjects) delete obj;
    }

    std::vector<purrObject*> getObjects() const { return mObjects; }

    void addObject(purrObject *object) {
      mObjects.push_back(object);
    }

    void removeObject(UUID *uuid) {
      auto it = std::find_if(mObjects.begin(), mObjects.end(), [uuid](purrObject *obj) 
        { return obj->getUuid() == uuid; });
      if (it == mObjects.end()) return;
      mObjects.erase(it);
      delete mObjects.at(mObjects.end() - it);
    }

    purrObject *getObject(UUID *uuid) {
      purrObject *obj = nullptr;
      auto it = std::find_if(mObjects.begin(), mObjects.end(), [uuid](purrObject *obj) 
        { return (obj->getUuid(), uuid) == 0; });
      if (it != mObjects.end()) obj = mObjects.at(mObjects.end() - it);
      return obj;
    }

    bool hasObject(UUID *uuid) {
      auto it = std::find_if(mObjects.begin(), mObjects.end(), [uuid](purrObject *obj) 
        { return (obj->getUuid(), uuid) == 0; });
      return it != mObjects.end();
    }
  private:
    std::vector<purrObject*> mObjects{};
  };

}

#endif