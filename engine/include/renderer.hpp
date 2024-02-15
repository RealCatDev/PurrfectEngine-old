#ifndef PURRENGINE_EDITOR_RENDERER_HPP_
#define PURRENGINE_EDITOR_RENDERER_HPP_

#include <PurrfectEngine/renderer.hpp>

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

      mCommands = new vkCommandPool(mRenderer);
      mCommands->initialize();
      mCommandBuffers = mCommands->allocate(MAX_FRAMES_IN_FLIGHT);

      mRenderPass = new vkRenderPass(mRenderer);
      auto attachment = vkRenderPass::attachmnetInfo();
      attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
      mRenderPass->addAttachment(attachment);
      mRenderPass->initialize();

      mRenderer->setSizeCallback([this](){ Resize(); });
      CreateSwapchain();

      mMesh = new vkMesh(mRenderer,
        {
          {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }},
          {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }},
          {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }},
          {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }}
        },
        {
          0, 1, 2, 2, 3, 0
        }
      );
      mMesh->initialize(mCommands);
    }

    void render() {
      mRenderer->beginDraw();
      auto cmdBuf = mCommandBuffers[mRenderer->frame()];
      vkResetCommandBuffer(cmdBuf, 0);
      RecordCommandBuffer(cmdBuf);
      mRenderer->endDraw(cmdBuf);
    }

    void cleanup() {
      CleanupSwapchain();
  
      delete mMesh;
      delete mCommands;
      delete mRenderPass;
    }
  private:
    void CreateSwapchain() {
      mSwapchain = new vkSwapchain(mRenderer);
      mSwapchain->chooseSwapSurfaceFormat({ VK_FORMAT_B8G8R8A8_UNORM }, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
      mSwapchain->chooseSwapPresentMode({ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR });
      mSwapchain->chooseSwapExtent(mWindow);
      mSwapchain->initialize();
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

      mPipeline = new vkPipeline(mRenderer);
      mPipeline->setRenderPass(mRenderPass);
      mPipeline->setVertexBind(MeshVertex::getBindingDescription());
      mPipeline->setVertexAttrs(MeshVertex::getAttributeDescriptions());
      auto vertBuf = new vkShader(mRenderer);
      auto fragBuf = new vkShader(mRenderer);
      vertBuf->load("../assets/shaders/vert.spv");
      fragBuf->load("../assets/shaders/frag.spv");
      mPipeline->addShader(VK_SHADER_STAGE_VERTEX_BIT,   vertBuf);
      mPipeline->addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragBuf);
      mPipeline->initialize();
    }

    void CleanupSwapchain() {
      for (size_t i = 0; i < mFramebuffers.size(); ++i) delete mFramebuffers[i];
      delete mPipeline;
      delete mSwapchain;
    }

    void Resize() {
      CleanupSwapchain();
      CreateSwapchain();
    }

    void RecordCommandBuffer(VkCommandBuffer cmdBuf) {
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      CHECK_VK(vkBeginCommandBuffer(cmdBuf, &beginInfo));

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

      vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->get());

      VkViewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (float) mSwapchain->getExtent().width;
      viewport.height = (float) mSwapchain->getExtent().height;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;
      vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

      VkRect2D scissor{};
      scissor.offset = {0, 0};
      scissor.extent = mSwapchain->getExtent();
      vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

      mMesh->render(cmdBuf);

      vkCmdEndRenderPass(cmdBuf);

      CHECK_VK(vkEndCommandBuffer(cmdBuf));
    }
  private:
    window        *mWindow = nullptr;

    vkRenderer    *mRenderer;
    vkSwapchain   *mSwapchain;
    vkCommandPool *mCommands;

    std::vector<vkFramebuffer*> mFramebuffers{};
    vkRenderPass *mRenderPass = nullptr;
    vkPipeline   *mPipeline   = nullptr;

    std::vector<VkCommandBuffer> mCommandBuffers{};

    vkMesh *mMesh = nullptr;
  };

}

#endif