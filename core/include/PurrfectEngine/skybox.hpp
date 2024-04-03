#ifndef   PURRENGINE_SKYBOX_HPP_
#define   PURRENGINE_SKYBOX_HPP_

#include "PurrfectEngine/texture.hpp"
#include "PurrfectEngine/cubemap.hpp"

#include <vulkan/vulkan.h>

namespace PurrfectEngine {

  struct SkyVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 normal;

    static VkVertexInputBindingDescription *getBindingDescription() {
      VkVertexInputBindingDescription *bindingDescription = new VkVertexInputBindingDescription();
      bindingDescription->binding = 0;
      bindingDescription->stride = sizeof(SkyVertex);
      bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(SkyVertex, position);

      return attributeDescriptions;
    }

    bool operator==(const SkyVertex& other) const {
      return position == other.position;
    }
  };

  class vkRenderPass;
  class vkSkybox {
  public:
    vkSkybox(vkRenderer *renderer);
    vkSkybox(vkRenderer *renderer, vkTexture *texture);
    vkSkybox(vkRenderer *renderer, const char *filepath);
    ~vkSkybox();

    void loadTexture(const char *filepath);

    void initialize(vkCommandPool *pool, vkDescriptorPool *descriptors);
    void cleanup();

    void draw(VkCommandBuffer cmdBuf, vkDescriptorSet *cameraSet);
  public:
    inline static vkPipeline *Pipeline() { return sPipeline; } // vkSkybox::Pipeline();

    static void Initialize(vkRenderer *renderer, vkRenderPass *renderPass);
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    inline static vkPipeline *sPipeline = nullptr; // Skybox pipeline
    inline static vkRenderPass *sRenderPass = nullptr; // Cubemaps render pass
    inline static vkPipeline *sCMPipeline = nullptr; // Cubemap pipeline
  private:
    vkTexture *mTexture = nullptr;
  };

}

#endif // PURRENGINE_SKYBOX_HPP_