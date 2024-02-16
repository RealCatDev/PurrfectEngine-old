#include "PurrfectEngine/imgui.hpp"

namespace PurrfectEngine {

  ImGuiHelper::ImGuiHelper(vkRenderer *renderer):
    mRenderer(renderer)
  {}

  ImGuiHelper::~ImGuiHelper() {
    cleanup();
  }

  void ImGuiHelper::initialize(window *win, vkDescriptorPool *descriptors, vkRenderPass *renderPass) {
    IMGUI_CHECKVERSION();
    ::ImGui::CreateContext();
    ImGuiIO& io = ::ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ::ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(win->get(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance        = mRenderer->mInstance;
    init_info.PhysicalDevice  = mRenderer->mPhysicalDevice;
    init_info.Device          = mRenderer->mDevice;
    init_info.QueueFamily     = mRenderer->mQueueFamily.graphicsFamily.value();
    init_info.Queue           = mRenderer->mGraphicsQueue;
    init_info.PipelineCache   = nullptr;
    init_info.DescriptorPool  = descriptors->get();
    init_info.Subpass         = 0;
    init_info.MinImageCount   = 2;
    init_info.ImageCount      = 3;
    init_info.MSAASamples     = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator       = nullptr;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, renderPass->get());
  }

  void ImGuiHelper::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ::ImGui::DestroyContext();
  }

  void ImGuiHelper::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ::ImGui::NewFrame();
  }

  void ImGuiHelper::render(VkCommandBuffer cmdBuf) {
    ::ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(::ImGui::GetDrawData(), cmdBuf);
  }

  void ImGuiHelper::update() {
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
    }
  }

}