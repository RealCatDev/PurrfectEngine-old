#ifndef   PURRENGINE_LIGHT_HPP_
#define   PURRENGINE_LIGHT_HPP_

namespace PurrfectEngine {

  struct vkLight {
    glm::vec4 pos;
    glm::vec4 col;
    glm::vec4 ambient;
  };

  struct vkLights {
    int lightCount;
    vkLight *lights;
  };

}

#endif // PURRENGINE_LIGHT_HPP_