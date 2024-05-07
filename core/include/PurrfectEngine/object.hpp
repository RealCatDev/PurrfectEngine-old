#ifndef PURRENGINE_OBJECT_HPP_
#define PURRENGINE_OBJECT_HPP_

#include "PurrfectEngine/components.hpp"

#include <algorithm>

namespace PurrfectEngine {

  class purrObject {
  public:
      purrObject(const char *name)
          : mName(name), mUUID(new UUID())
      { addComponent(new transformComponent(new purrTransform())); }

      ~purrObject() {
          for (auto com : mComponents) delete com;
          free((void*)mName);
      }

      bool addComponent(purrComponent *component) {
          if (getComponent(component->getName()) != NULL) return false;
          mComponents.push_back(component);
          return true;
      }

      const std::vector<purrComponent*>& getComponents() const {
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
          { return strcmp(c->getName(), name) == 0; });
          if (it == mComponents.end()) return;
          delete *it;
          mComponents.erase(it);
      }

      UUID *getUuid() const { return mUUID; }
      const std::string& getName() const { return mName; }

      const std::vector<purrObject*>& getChildren() const { return mChildren; }

      void addChild(purrObject* child) {
          mChildren.push_back(child);
      }

      void removeChild(purrObject* child) {
          auto it = std::find(mChildren.begin(), mChildren.end(), child);
          if (it != mChildren.end()) {
              mChildren.erase(it);
          }
      }

  private:
      UUID                       *mUUID;
      const char                 *mName;
      std::vector<purrComponent*> mComponents;
      std::vector<purrObject*>    mChildren;
  };
}

#endif