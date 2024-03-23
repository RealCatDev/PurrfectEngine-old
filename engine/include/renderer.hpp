#ifndef PURRENGINE_EDITOR_RENDERER_HPP_
#define PURRENGINE_EDITOR_RENDERER_HPP_

#include <PurrfectEngine/renderer.hpp>
#include <PurrfectEngine/camera.hpp>
#include <PurrfectEngine/imgui.hpp>

#include <glm/gtc/type_ptr.hpp>

#include "panels.hpp"

namespace PurrfectEngine {

  class renderer {
  public:
    renderer(window *win):
      mRenderer(new vkRenderer(win)), mWindow(win)
    {}

    ~renderer() {
      cleanup();

      delete mRenderer;
    }

    void initialize() {
      mRenderer->initialize();

      (void)getTextureLayout(mRenderer);

      mCameraLayout = new vkDescriptorLayout(mRenderer);
      mCameraLayout->addBinding({
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr
      });
      mCameraLayout->initialize();

      mCommands = new vkCommandPool(mRenderer);
      mCommands->initialize();
      mCommandBuffers = mCommands->allocate(MAX_FRAMES_IN_FLIGHT);

      mDescriptors = new vkDescriptorPool(mRenderer);
      mDescriptors->initialize({
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048 }
      });
      mCameraSet = mDescriptors->allocate(mCameraLayout);

      mRenderPass = new vkRenderPass(mRenderer);
      auto attachment = vkRenderPass::attachmnetInfo();
      attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
      mRenderPass->addAttachment(attachment);
      mRenderPass->initialize();

      mSceneRenderPass = new vkRenderPass(mRenderer);
      {
        attachment = vkRenderPass::attachmnetInfo();
        attachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachment.msaaSamples = true;
        mSceneRenderPass->addAttachment(attachment);
      }
      
      {
        attachment = vkRenderPass::attachmnetInfo();
        attachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        mSceneRenderPass->addAttachmentResolve(attachment);
      }
      mSceneRenderPass->initialize();

      mRenderer->setSizeCallback([this](){ Resize(); });
      mRenderer->setResizeCheck([this]() {
        if (mSceneResized) {
          if (!mSwapchainResized) vkDeviceWaitIdle(mRenderer->getDevice());
          ResizeScene();
          mSceneResized = false;
        }
        mSwapchainResized = false;
      });

      mCameraBuf = new vkBuffer(mRenderer);
      mCameraBuf->initialize(sizeof(CameraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      mCameraBuf->mapMemory();

      mCameraSet->write(mCameraBuf);

      mSceneExtent = { 1920, 1080 };
      CreateSwapchain();
      CreateScene();

      mImGuiDescriptors = new vkDescriptorPool(mRenderer);
      mImGuiDescriptors->initialize({
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
      });

      mImGui = new ImGuiHelper(mRenderer);
      mImGui->initialize(mWindow, mImGuiDescriptors, mRenderPass);

      mTexture = new vkTexture(mRenderer, Asset("textures/texture.png"));
      mTexture->initialize(mCommands, mDescriptors, mSwapchain->getFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      mCamera = new purrCamera(new purrTransform(glm::vec3(0.0f, 0.0f, -10.0f)));
    }

    void render() {
      if (mRenderer->beginDraw()) {
        auto cmdBuf = mCommandBuffers[mRenderer->frame()];
        vkResetCommandBuffer(cmdBuf, 0);
        Update();
        RecordCommandBuffer(cmdBuf);
        mRenderer->endDraw(cmdBuf);
      }
    }

    void cleanup() {
      CleanupSwapchain();
      CleanupScene();
  
      delete mImGui;

      delete mCameraLayout;
      destroyTextureLayout(); // destroy texture layout

      delete mCameraBuf;
      delete mTexture;
      delete mCommands;
      delete mDescriptors;
      delete mImGuiDescriptors;
      delete mRenderPass;
      delete mSceneRenderPass;
    }

    void setScene(purrScene *scene) { mScene = scene; }

    PurrfectEngine::vkMesh *createMesh(std::vector<MeshVertex> vertices, std::vector<uint32_t> indices) {
      auto mesh = new vkMesh(mRenderer,
        vertices, indices
      );
      mesh->initialize(mCommands);
      return mesh;
    }
  private:
    void Update() {
      auto extnt = mSwapchain->getExtent();
      mCamera->calculate({ extnt.width, extnt.height });
      // Update CameraUBO
      {
        CameraUBO ubo{};
        ubo.proj = mCamera->getProjection();
        ubo.view = mCamera->getView();

        mCameraBuf->setData((void*)&ubo);
      }
    }

    void CreateSwapchain() {
      mSwapchain = new vkSwapchain(mRenderer);
      mSwapchain->chooseSwapSurfaceFormat({ VK_FORMAT_B8G8R8A8_UNORM }, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
      mSwapchain->chooseSwapPresentMode({ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR });
      mSwapchain->chooseSwapExtent(mWindow);
      mSwapchain->setLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
      mSwapchain->initialize(mCommands);
      mSwapchain->attach(mRenderer);

      mFramebuffers.resize(MAX_FRAMES_IN_FLIGHT);
      for (size_t i = 0; i < mFramebuffers.size(); ++i) {
        auto fb = new vkFramebuffer(mRenderer);
        fb->setRenderPass(mRenderPass);
        fb->setExtent(mSwapchain->getExtent());
        fb->addAttachment(mSwapchain->getView(i));
        fb->initialize();

        mFramebuffers[i] = fb;
      }
    }

    void CleanupSwapchain() {
      for (size_t i = 0; i < mFramebuffers.size(); ++i)      delete mFramebuffers[i];
      delete mSwapchain;
    }

    void Resize() {
      CleanupSwapchain();
      CreateSwapchain();
      mSwapchainResized = true;
    }

    void CreateScene() {
      mSceneImages.clear();
      mSceneResolveImages.clear();
      mSceneFramebuffers.clear();
      for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkTexture *resolveTexture = new vkTexture(mRenderer);
        vkTexture *texture = new vkTexture(mRenderer);
        auto fb = new vkFramebuffer(mRenderer);
        fb->setRenderPass(mSceneRenderPass);
        {
          resolveTexture->initialize(mCommands, mDescriptors, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, mSceneExtent.width, mSceneExtent.height, false, true, false);
          fb->addAttachment(resolveTexture->getView());
        }
        {
          texture->initialize(mCommands, mDescriptors, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mSceneExtent.width, mSceneExtent.height, false);
          fb->addAttachment(texture->getView());
        }
        fb->setExtent(mSceneExtent);
        fb->initialize();
        
        mSceneImages.push_back(texture);
        mSceneResolveImages.push_back(resolveTexture);
        mSceneFramebuffers.push_back(fb);
      }

      mPipeline = new vkPipeline(mRenderer);
      mPipeline->setRenderPass(mSceneRenderPass);
      mPipeline->addDescriptor(mCameraLayout);
      mPipeline->addDescriptor(getTextureLayout());
      mPipeline->setVertexBind(MeshVertex::getBindingDescription());
      mPipeline->setVertexAttrs(MeshVertex::getAttributeDescriptions());

      auto vertBuf = new vkShader(mRenderer);
      auto fragBuf = new vkShader(mRenderer);
      vertBuf->load(Asset("shaders/vert.spv"));
      fragBuf->load(Asset("shaders/frag.spv"));
      mPipeline->addShader(VK_SHADER_STAGE_VERTEX_BIT,   vertBuf);
      mPipeline->addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragBuf);
      mPipeline->initialize();
    }

    void CleanupScene() {
      for (size_t i = 0; i < mSceneImages.size(); ++i)        delete mSceneImages[i];
      for (size_t i = 0; i < mSceneResolveImages.size(); ++i) delete mSceneResolveImages[i];
      for (size_t i = 0; i < mSceneFramebuffers.size(); ++i)  delete mSceneFramebuffers[i];
      delete mPipeline;
    }

    void ResizeScene() {
      CleanupScene();
      CreateScene();
    }

    void RecordCommandBuffer(VkCommandBuffer cmdBuf) {
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      CHECK_VK(vkBeginCommandBuffer(cmdBuf, &beginInfo));

      {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mSceneRenderPass->get();
        renderPassInfo.framebuffer = mSceneFramebuffers[mRenderer->frame()]->get();
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = mSceneExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) mSceneExtent.width;
        viewport.height = (float) mSceneExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = mSceneExtent;
        vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->get());

        if (mScene) {
          mCameraSet->bind(cmdBuf, mPipeline);
          mTexture->bind(cmdBuf, mPipeline); // Default texture

          for (auto obj : mScene->getObjects()) {
            if (!obj->hasComponent("Mesh")) continue;
            meshComponent *meshCmp = (meshComponent*) obj->getComponent("Mesh");
            meshCmp->get()->render(cmdBuf);
          }
        }

        vkCmdEndRenderPass(cmdBuf);
      }

      {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mRenderPass->get();
        renderPassInfo.framebuffer = mFramebuffers[mRenderer->frame()]->get();
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = mSwapchain->getExtent();

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        mImGui->newFrame();
        RenderImGui();
        mImGui->render(cmdBuf);

        vkCmdEndRenderPass(cmdBuf);
      }

      CHECK_VK(vkEndCommandBuffer(cmdBuf));
    }

    void RenderImGui() {
      {
        bool menuBar = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = (menuBar ? ImGuiWindowFlags_MenuBar : (ImGuiWindowFlags)0) | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        {
          static bool dockOpen = true;
          ImGui::Begin("DockSpace Demo", &dockOpen, window_flags);
        }
        
        ImGui::PopStyleVar(3);

        ImGuiIO& io = ImGui::GetIO();
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        if (menuBar && ImGui::BeginMenuBar()) {
          if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit", NULL, false)); // TODO(CatDev)
            ImGui::EndMenu();
          }

          ImGui::EndMenuBar();
        }
      }

      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
      ImGui::Begin("Scene", &mSceneWndoOpen, ImGuiWindowFlags_NoCollapse);

      auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
      auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
      auto viewportOffset = ImGui::GetWindowPos();
      mScnViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
      mScnViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

      auto regAval = ImGui::GetContentRegionAvail();
      if (glm::vec2(regAval.x, regAval.y) != glm::vec2(mSceneExtent.width, mSceneExtent.height)) {
        mSceneResized = true;
        mSceneExtent = { static_cast<uint32_t>(regAval.x), static_cast<uint32_t>(regAval.y) };
      }
      ImGui::Image(mSceneImages[mRenderer->frame()]->getSet(), { static_cast<float>(mSceneExtent.width), static_cast<float>(mSceneExtent.height) });

      ImGui::End();
      ImGui::PopStyleVar();

      HierarchyPanel::Render();

      ImGui::End(); // Dockspace
    }
  private:
    window           *mWindow      = nullptr;

    vkRenderer       *mRenderer    = nullptr;
    vkSwapchain      *mSwapchain   = nullptr;
    vkCommandPool    *mCommands    = nullptr;
    vkDescriptorPool *mDescriptors = nullptr;

    vkDescriptorLayout *mCameraLayout = nullptr;

    vkRenderPass *mRenderPass = nullptr;
    std::vector<vkFramebuffer*> mFramebuffers{};
    vkDescriptorPool *mImGuiDescriptors = nullptr;

    std::vector<VkCommandBuffer> mCommandBuffers{};
    bool                         mSwapchainResized = false;

    ImGuiHelper     *mImGui     = nullptr;

    glm::vec2 mScnViewportBounds[2];

    vkRenderPass    *mSceneRenderPass = nullptr;
    VkExtent2D       mSceneExtent = {};
    bool             mSceneResized = false;

    std::vector<vkTexture*>     mSceneImages{};
    std::vector<vkTexture*>     mSceneResolveImages{};
    std::vector<vkFramebuffer*> mSceneFramebuffers{};

    vkPipeline      *mPipeline  = nullptr;

    vkBuffer        *mCameraBuf = nullptr;
    vkDescriptorSet *mCameraSet = nullptr;
    vkMesh          *mMesh      = nullptr;
    vkTexture       *mTexture   = nullptr;

    purrCamera      *mCamera    = nullptr;

    purrScene       *mScene     = nullptr;
  private: // ImGui stuff
    bool mSceneWndoOpen = true;
  };

}

#endif