#ifndef PURRENGINE_MESH_HPP_
#define PURRENGINE_MESH_HPP_

#include "PurrfectEngine/core.hpp"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace PurrfectEngine {

  struct MeshVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 normal;

    static VkVertexInputBindingDescription getBindingDescription() {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(MeshVertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(MeshVertex, position);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(MeshVertex, color);

      attributeDescriptions[2].binding = 0;
      attributeDescriptions[2].location = 2;
      attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[2].offset = offsetof(MeshVertex, uv);

      attributeDescriptions[3].binding = 0;
      attributeDescriptions[3].location = 3;
      attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[3].offset = offsetof(MeshVertex, normal);

      return attributeDescriptions;
    }

    bool operator==(const MeshVertex& other) const {
      return position == other.position && color == other.color && uv == other.uv && normal == other.normal;
    }
  };

  class vkRenderer;
  class vkBuffer;
  class vkCommandPool;
  class vkMesh {
  public:
    vkMesh(vkRenderer *renderer);
    vkMesh(vkRenderer *renderer, std::vector<MeshVertex> vertices, std::vector<uint32_t> indices);
    ~vkMesh();

    void initialize(vkCommandPool *pool);
    void cleanup();
    void render(VkCommandBuffer cmdBuf);
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    vkBuffer *mVBuf = nullptr;
    vkBuffer *mIBuf = nullptr;
    std::vector<MeshVertex> mVertices; 
    std::vector<uint32_t>   mIndices;
  };

}

#endif