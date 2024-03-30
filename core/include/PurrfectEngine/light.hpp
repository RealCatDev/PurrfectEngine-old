#ifndef   PURRENGINE_LIGHT_HPP_
#define   PURRENGINE_LIGHT_HPP_

namespace PurrfectEngine {

  struct vkLight {
    glm::vec4 pos;
    glm::vec4 col;
  };

  struct vkLights {
    int lightCount;
    glm::vec4 ambient;
    vkLight *lights;
  };

}

#endif // PURRENGINE_LIGHT_HPP_