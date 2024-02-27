#ifndef PURRENGINE_PANELS_HPP_
#define PURRENGINE_PANELS_HPP_

#include <imgui.h>

namespace PurrfectEngine {

  class HierarchyPanel {
  public:
    static void Render() {
      ImGui::Begin("Hierarchy", &sOpen);



      ImGui::End();
    }
  private:
    inline static bool sOpen = true;
  };

}

#endif