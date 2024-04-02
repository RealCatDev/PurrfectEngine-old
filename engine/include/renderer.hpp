#ifndef PURRENGINE_EDITOR_RENDERER_HPP_
#define PURRENGINE_EDITOR_RENDERER_HPP_

#include <PurrfectEngine/renderer.hpp>
#include <PurrfectEngine/camera.hpp>
#include <PurrfectEngine/mesh.hpp>
#include <PurrfectEngine/light.hpp>
#include <PurrfectEngine/loaders.hpp>
#include <PurrfectEngine/imgui.hpp>
#include <PurrfectEngine/material.hpp>

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
      (void)getMaterialLayout(mRenderer);

      mCameraLayout = new vkDescriptorLayout(mRenderer);
      mCameraLayout->addBinding({
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
      });
      mCameraLayout->initialize();
      
      mLightsLayout = new vkDescriptorLayout(mRenderer);
      mLightsLayout->addBinding({
        0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
      });
      mLightsLayout->initialize();

      mCommands = new vkCommandPool(mRenderer);
      mCommands->initialize();
      mCommandBuffers = mCommands->allocate(MAX_FRAMES_IN_FLIGHT);

      mDescriptors = new vkDescriptorPool(mRenderer);
      mDescriptors->initialize({
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048 }
      });

      vkTexture::initializeBlank(mRenderer, mCommands, mDescriptors);

      mCameraSet = mDescriptors->allocate(mCameraLayout);
      mLightsSet = mDescriptors->allocate(mLightsLayout);

      mRenderPass = new vkRenderPass(mRenderer);
      auto attachment = vkRenderPass::attachmentInfo();
      attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
      mRenderPass->addAttachment(attachment);
      mRenderPass->initialize();

      {
        mSceneRenderPass = new vkRenderPass(mRenderer);
        {
          attachment = vkRenderPass::attachmentInfo();
          attachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
          attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
          attachment.msaaSamples = true;
          mSceneRenderPass->addAttachment(attachment);
        }
        
        {
          attachment = vkRenderPass::attachmentInfo();
          attachment.format = mRenderer->getDepthFormat();
          attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
          attachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
          attachment.msaaSamples = true;
          mSceneRenderPass->addAttachment(attachment);
        }

        {
          attachment = vkRenderPass::attachmentInfo();
          attachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
          attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
          attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          mSceneRenderPass->addAttachmentResolve(attachment);
        }
        mSceneRenderPass->initialize();
      }

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

      mLights.push_back({
        glm::vec4(0.0f, 0.0f, 10.0f, 0.0f),
        glm::vec4(1.0f, 1.0f, 1.0f, 150.0f)
      });
      CreateLightsBuf();

      mSceneExtent = { 1920, 1080 };
      CreateSwapchain();
      {
        mHdrRenderPass = new vkRenderPass(mRenderer);
        {
          attachment = vkRenderPass::attachmentInfo();
          attachment.format = mSwapchain->getFormat();
          attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
          mHdrRenderPass->addAttachment(attachment);
        }
        mHdrRenderPass->initialize();
      }
      CreateScene();

      mImGuiDescriptors = new vkDescriptorPool(mRenderer);
      mImGuiDescriptors->initialize({
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
      });

      mImGui = new ImGuiHelper(mRenderer);
      mImGui->initialize(mWindow, mImGuiDescriptors, mRenderPass);

      mTexture = new vkTexture(mRenderer, Asset("textures/texture.png"));
      mTexture->initialize(mCommands, mDescriptors, mSwapchain->getFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      {
        vkTexture *albedo    = new vkTexture(mRenderer, Asset("textures/rustediron/albedo.png"));
        vkTexture *normal    = new vkTexture(mRenderer, Asset("textures/rustediron/normal.png"));
        vkTexture *metallic  = new vkTexture(mRenderer, Asset("textures/rustediron/metallic.png"));
        vkTexture *roughness = new vkTexture(mRenderer, Asset("textures/rustediron/roughness.png"));
        vkTexture *ao        = new vkTexture(mRenderer, Asset("textures/rustediron/ao.png"));
        albedo->initialize(mCommands, mDescriptors, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, -1, -1, true, false, false);
        normal->initialize(mCommands, mDescriptors, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, -1, -1, true, false, false);
        metallic->initialize(mCommands, mDescriptors, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, -1, -1, true, false, false);
        roughness->initialize(mCommands, mDescriptors, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, -1, -1, true, false, false);
        ao->initialize(mCommands, mDescriptors, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, -1, -1, true, false, false);
        mRustIronMat         = new vkMaterial(mRenderer, albedo);
        mRustIronMat->setNormal(normal);
        mRustIronMat->setMetallic(metallic);
        mRustIronMat->setRoughness(roughness);
        mRustIronMat->setAmbientOcclusion(ao);
        mRustIronMat->initialize(mDescriptors);
      }

      mCamera = new purrCamera(new purrTransform(glm::vec3(0.0f, 0.0f, -10.0f)));

      (void)new modelLoader(mRenderer);
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
      delete mLightsLayout;
      destroyTextureLayout(); // destroy texture layout

      delete mLightsBuf;
      delete mCameraBuf;
      delete mRustIronMat;
      delete mTexture;
      delete mCommands;
      delete mDescriptors;
      delete mImGuiDescriptors;
      delete mRenderPass;
      delete mSceneRenderPass;
    }

    void setScene(purrScene *scene) { mScene = scene; }

    PurrfectEngine::vkMesh *createMesh(const char *filepath) {
      vkMesh *mesh = PurrfectEngine::modelLoader::load(filepath, mCommands);
      PURR_ASSERT(mesh, "Failed to load mesh!");
      return mesh;
    }
  private:
    void CreateLightsBuf() {
      int lightCount = static_cast<int>(mLights.size());
      VkDeviceSize size = (sizeof(int)*4) + (sizeof(vkLight) * lightCount);
      auto stagingBuf = new vkBuffer(mRenderer);
      stagingBuf->initialize(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      glm::vec4 ambient(1.0f, 1.0f, 1.0f, 0.2f);
      stagingBuf->mapMemory();
        void* data = stagingBuf->getData();
        memcpy(data, &lightCount, sizeof(int)*4);
        memcpy(&(((char*)data)[sizeof(glm::vec4)]),  &ambient,        sizeof(glm::vec4));
        memcpy(&(((char*)data)[sizeof(glm::vec4)*2]), mLights.data(), (sizeof(vkLight)*lightCount));
      stagingBuf->unmapMemory();

      mLightsBuf = new vkBuffer(mRenderer);
      mLightsBuf->initialize(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      mLightsBuf->copy(mCommands, stagingBuf);

      delete stagingBuf;

      mLightsSet->write(mLightsBuf, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    }

    void RecreateLightsBuf() {
      delete mLightsBuf;
      CreateLightsBuf();
    }

    void Update() {
      auto extnt = mSwapchain->getExtent();
      mCamera->calculate({ extnt.width, extnt.height });
      // Update CameraUBO
      {
        CameraUBO ubo{};
        ubo.proj = mCamera->getProjection();
        ubo.view = mCamera->getView();
        ubo.pos  = mCamera->getPosition();

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
      mHdrImages.clear();
      mHdrFramebuffers.clear();
      mSceneImages.clear();
      mSceneResolveImages.clear();
      mSceneDepthImages.clear();
      mSceneFramebuffers.clear();
      for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        {
          vkTexture *resolveTexture = new vkTexture(mRenderer);
          vkTexture *texture = new vkTexture(mRenderer);
          vkTexture *depthTexture = new vkTexture(mRenderer);
          auto fb = new vkFramebuffer(mRenderer);
          fb->setRenderPass(mSceneRenderPass);
          {
            resolveTexture->initialize(mCommands, mDescriptors, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, mSceneExtent.width, mSceneExtent.height, false, true, false);
            fb->addAttachment(resolveTexture->getView());
          }
          {
            depthTexture->initialize(mCommands, mDescriptors, mRenderer->getDepthFormat(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, mSceneExtent.width, mSceneExtent.height, false, true, false);
            fb->addAttachment(depthTexture->getView());
          }
          {
            texture->initialize(mCommands, mDescriptors, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mSceneExtent.width, mSceneExtent.height, false);
            fb->addAttachment(texture->getView());
          }
          fb->setExtent(mSceneExtent);
          fb->initialize();
          
          mSceneImages.push_back(texture);
          mSceneResolveImages.push_back(resolveTexture);
          mSceneDepthImages.push_back(depthTexture);
          mSceneFramebuffers.push_back(fb);
        }

        {
          vkTexture *texture = new vkTexture(mRenderer);
          texture->initialize(mCommands, mDescriptors, mSwapchain->getFormat(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mSceneExtent.width, mSceneExtent.height, false);
          mHdrImages.push_back(texture);
          auto fb = new vkFramebuffer(mRenderer);
          fb->setRenderPass(mHdrRenderPass);
          fb->addAttachment(texture->getView());
          fb->setExtent(mSceneExtent);
          fb->initialize();
          mHdrFramebuffers.push_back(fb);
        }
      }

      { // lit pipeline
        mPipeline = new vkPipeline(mRenderer);
        mPipeline->setRenderPass(mSceneRenderPass);
        mPipeline->addDescriptor(mCameraLayout);
        mPipeline->addDescriptor(getMaterialLayout());
        mPipeline->addDescriptor(mLightsLayout);
        mPipeline->addPushConstant(0, sizeof(glm::mat4)*2, VK_SHADER_STAGE_VERTEX_BIT);
        mPipeline->setVertexBind(MeshVertex::getBindingDescription());
        mPipeline->setVertexAttrs(MeshVertex::getAttributeDescriptions());
        mPipeline->enableDepthStencil(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE, VK_FALSE);

        auto vertShader = new vkShader(mRenderer);
        auto fragShader = new vkShader(mRenderer);
        vertShader->load(Asset("shaders/vert.spv"));
        fragShader->load(Asset("shaders/pbr.spv"));
        mPipeline->addShader(VK_SHADER_STAGE_VERTEX_BIT,   vertShader);
        mPipeline->addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader);
        mPipeline->setCulling(VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_CULL_MODE_BACK_BIT);
        mPipeline->enableMSAA();
        mPipeline->initialize();
      }

      { // hdr pipeline
        mHdrPipeline = new vkPipeline(mRenderer);
        mHdrPipeline->setRenderPass(mHdrRenderPass);
        mHdrPipeline->addDescriptor(getTextureLayout());

        auto vertShader = new vkShader(mRenderer);
        auto fragShader = new vkShader(mRenderer);
        vertShader->load(Asset("shaders/plane.spv"));
        fragShader->load(Asset("shaders/hdr.spv"));
        mHdrPipeline->addShader(VK_SHADER_STAGE_VERTEX_BIT,   vertShader);
        mHdrPipeline->addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader);
        mHdrPipeline->setCulling(VK_FRONT_FACE_CLOCKWISE, VK_CULL_MODE_BACK_BIT);
        mHdrPipeline->initialize();
      }
    }

    void CleanupScene() {
      for (size_t i = 0; i < mHdrImages.size(); ++i)          delete mHdrImages[i];
      for (size_t i = 0; i < mHdrFramebuffers.size(); ++i)    delete mHdrFramebuffers[i];
      for (size_t i = 0; i < mSceneImages.size(); ++i)        delete mSceneImages[i];
      for (size_t i = 0; i < mSceneResolveImages.size(); ++i) delete mSceneResolveImages[i];
      for (size_t i = 0; i < mSceneDepthImages.size(); ++i)   delete mSceneDepthImages[i];
      for (size_t i = 0; i < mSceneFramebuffers.size(); ++i)  delete mSceneFramebuffers[i];
      delete mPipeline;
      delete mHdrPipeline;
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

        std::vector<VkClearValue> clearValues(3);
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        clearValues[2].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

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
          mRustIronMat->bind(cmdBuf, mPipeline, 1);
          mLightsSet->bind(cmdBuf, mPipeline, 2);

          for (auto obj : mScene->getObjects()) {
            if (!obj->hasComponent("Mesh")) continue;
            transformComponent *transform = (transformComponent*) obj->getComponent("Transform");
            PURR_ASSERT(transform, "Transform is required to render game object!");

            vkModelPC pc{};
            pc.model = transform->get()->getTransform();
            pc.normal = glm::mat4(transform->get()->getNormal());
            vkCmdPushConstants(cmdBuf, mPipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vkModelPC), &pc);

            meshComponent *meshCmp = (meshComponent*) obj->getComponent("Mesh");
            meshCmp->get()->render(cmdBuf);
          }
        }

        vkCmdEndRenderPass(cmdBuf);
      }

      {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mHdrRenderPass->get();
        renderPassInfo.framebuffer = mHdrFramebuffers[mRenderer->frame()]->get();
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = mSceneExtent;

        std::vector<VkClearValue> clearValues(1);
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

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

        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mHdrPipeline->get());

        mSceneImages[mRenderer->frame()]->bind(cmdBuf, mHdrPipeline, 0);
        vkCmdDraw(cmdBuf, 6, 1, 0, 0);

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
      ImGui::Image(mHdrImages[mRenderer->frame()]->getSet(), { static_cast<float>(mSceneExtent.width), static_cast<float>(mSceneExtent.height) });

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
    vkDescriptorLayout *mLightsLayout = nullptr;

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

    vkRenderPass               *mHdrRenderPass = nullptr;
    std::vector<vkTexture*>     mHdrImages{};
    std::vector<vkFramebuffer*> mHdrFramebuffers{};

    std::vector<vkTexture*>     mSceneImages{};
    std::vector<vkTexture*>     mSceneResolveImages{};
    std::vector<vkTexture*>     mSceneDepthImages{};
    std::vector<vkFramebuffer*> mSceneFramebuffers{};

    vkPipeline      *mPipeline  = nullptr;
    vkPipeline      *mHdrPipeline  = nullptr;

    vkBuffer        *mCameraBuf   = nullptr;
    vkDescriptorSet *mCameraSet   = nullptr;
    vkMesh          *mMesh        = nullptr;
    vkTexture       *mTexture     = nullptr;
    vkMaterial      *mRustIronMat = nullptr;

    vkBuffer        *mLightsBuf = nullptr;
    vkDescriptorSet *mLightsSet = nullptr;

    purrCamera      *mCamera    = nullptr;

    purrScene       *mScene     = nullptr;

    std::vector<vkLight> mLights;
  private: // ImGui stuff
    bool mSceneWndoOpen = true;
  };

}

#endif