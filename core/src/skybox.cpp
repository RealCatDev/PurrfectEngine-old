#include "PurrfectEngine/skybox.hpp"

#include "PurrfectEngine/renderer.hpp"

namespace PurrfectEngine {

  vkSkybox::vkSkybox(vkRenderer *renderer):
    mRenderer(renderer) 
  {}

  vkSkybox::vkSkybox(vkRenderer *renderer, vkTexture *texture):
    mRenderer(renderer), mTexture(texture) 
  {}

  vkSkybox::vkSkybox(vkRenderer *renderer, const char *filepath):
    mRenderer(renderer), mTexture(new vkTexture(renderer, filepath))
  {}

  vkSkybox::~vkSkybox() {
    cleanup();
  }

  void vkSkybox::loadTexture(const char *filepath) {
    if (mTexture) delete mTexture;
    mTexture = new vkTexture(mRenderer, filepath);
  }

  void vkSkybox::initialize(vkCommandPool *pool, vkDescriptorPool *descriptors) {
    PURR_ASSERT(sPipeline);
    PURR_ASSERT(mTexture);
    if (!mTexture->getImage()) mTexture->initialize(pool, descriptors, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    {
      auto cmdBuf = pool->beginSingleTimeCommands();
      VkRenderPassBeginInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = sRenderPass->get();
      renderPassInfo.renderArea.offset = {0, 0};
      renderPassInfo.renderArea.extent = VkExtent2D(mWidth, mHeight, 1);
      VkClearValue clearValue = {{0.0f, 0.0f, 0.0f, 1.0f}};
      renderPassInfo.clearValueCount = 1;
      renderPassInfo.pClearValues = &clearValue;

      glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
      glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
      };

      for (uint8_t i = 0; i < 6; ++i) {
        renderPassInfo.framebuffer = mFramebuffers[i]->get();
        vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, sPipeline->get());
        

        vkCmdEndRenderPass(cmdBuf);
      }
      pool->endSingleTimeCommands(cmdBuf);
    }
  }

  void vkSkybox::cleanup() {
    delete mTexture;
  }

  void vkSkybox::draw(VkCommandBuffer cmdBuf, vkDescriptorSet *cameraSet) {
    PURR_ASSERT(cameraSet);
    PURR_ASSERT(mSet, "mSet is not initialized! Did you forgor to call vkSkybox::initialize()?");

    
  }

  void vkSkybox::Initialize(vkRenderer *renderer, vkRenderPass *renderPass) {
    if (sPipeline) {
      delete sPipeline;
      delete sRenderPass;
      delete sCMPipeline;
    } else {
      {
        sPipeline = new vkPipeline(renderer);
        sPipeline->setRenderPass(renderPass);
        sPipeline->addDescriptor(mCameraLayout);
        sPipeline->addDescriptor(getTextureLayout());
        sPipeline->setVertexBind(SkyVertex::getBindingDescription());
        sPipeline->setVertexAttrs(SkyVertex::getAttributeDescriptions());
        sPipeline->enableDepthStencil(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE, VK_FALSE);

        auto vertShader = new vkShader(renderer);
        auto fragShader = new vkShader(renderer);
        vertShader->load(Asset("shaders/skyVert.spv"));
        fragShader->load(Asset("shaders/skyFrag.spv"));
        sPipeline->addShader(VK_SHADER_STAGE_VERTEX_BIT,   vertShader);
        sPipeline->addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader);
        sPipeline->setCulling(VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_CULL_MODE_BACK_BIT);
        sPipeline->initialize();
      }

      {
        sRenderPass = new vkRenderPass(renderer);
        {
          vkRenderPass::attachmentInfo info{};
          info.finalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
          info.format        = VK_FORMAT_R16G16B16A16_SFLOAT;
          sRenderPass->addAttachment(info);
        }
        sRenderPass->initialize();
      }
      
      {
        sCMPipeline = new vkPipeline(renderer);
        sCMPipeline->setRenderPass(sRenderPass);
        sCMPipeline->addDescriptor(mCameraLayout);
        sCMPipeline->addDescriptor(getTextureLayout());
        sCMPipeline->setVertexBind(SkyVertex::getBindingDescription());
        sCMPipeline->setVertexAttrs(SkyVertex::getAttributeDescriptions());
        sCMPipeline->enableDepthStencil(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE, VK_FALSE);

        auto vertShader = new vkShader(renderer);
        auto fragShader = new vkShader(renderer);
        vertShader->load(Asset("shaders/skyVert.spv"));
        fragShader->load(Asset("shaders/skyFrag.spv"));
        sCMPipeline->addShader(VK_SHADER_STAGE_VERTEX_BIT,   vertShader);
        sCMPipeline->addShader(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader);
        sCMPipeline->setCulling(VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_CULL_MODE_BACK_BIT);
        sCMPipeline->initialize();
      }
    }
  }

}