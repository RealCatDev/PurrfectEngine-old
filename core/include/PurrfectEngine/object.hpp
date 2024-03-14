#ifndef PURRENGINE_OBJECT_HPP_
#define PURRENGINE_OBJECT_HPP_

#include "PurrfectEngine/components.hpp"

#include <algorithm>

namespace PurrfectEngine {

  class purrObject {
  public:
    purrObject(const char *name) 
      : mName(name), mUUID(new UUID())
    {}

    ~purrObject() {
      for (auto com : mComponents) delete com;
      free((void*)mName);
    }

    bool addComponent(purrComponent *component) {
      if (getComponent(component->getName()) != NULL) return false;
      mComponents.push_back(component);
      return true;
    }

    std::vector<purrComponent*> getComponents() {
      return mComponents;
    }

    purrComponent* getComponent(const char *name) {
      for (auto comp : mComponents)
        if (strcmp(comp->getName(), name) == 0) return comp;
      return nullptr;
    }

    bool hasComponent(const char *name) {
      return getComponent(name) != nullptr;
    }

    void removeComponent(const char *name) {
      auto it = std::find_if(mComponents.begin(), mComponents.end(), [name](purrComponent *c) 
        { return (c->getName(), name) == 0; });
      if (it == mComponents.end()) return;
      mComponents.erase(it);
      delete mComponents.at(mComponents.end() - it);
    }

    UUID *getUuid() const { return mUUID; }
  private:
    UUID                       *mUUID;
    const char                 *mName;
    std::vector<purrComponent*> mComponents;
  };

}

#endif