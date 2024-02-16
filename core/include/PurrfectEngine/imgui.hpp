#ifndef PURRENGINE_IMGUI_HPP_
#define PURRENGINE_IMGUI_HPP_

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "PurrfectEngine/renderer.hpp"
#include "PurrfectEngine/window.hpp"

namespace PurrfectEngine {

  class ImGuiHelper {
  public:
    ImGuiHelper(vkRenderer *renderer);
    ~ImGuiHelper();

    void initialize(window *win, vkDescriptorPool *descriptors, vkRenderPass *renderPass);
    void cleanup();

    void newFrame();
    void render(VkCommandBuffer cmdBuf);
    
    static void update();
  private:
    vkRenderer *mRenderer = nullptr;
  };

}

#endif