#ifndef PURRENGINE_MESH_HPP_
#define PURRENGINE_MESH_HPP_

#include "PurrfectEngine/core.hpp"

#include <vulkan/vulkan.h>

namespace PurrfectEngine {

  struct MeshVertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 normal;

    static VkVertexInputBindingDescription *getBindingDescription() {
      VkVertexInputBindingDescription *bindingDescription = new VkVertexInputBindingDescription();
      bindingDescription->binding = 0;
      bindingDescription->stride = sizeof(MeshVertex);
      bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

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

  struct vkModelPC {
    glm::mat4 model;
    glm::mat4 normal;
  };

  struct vkSubMesh {
    unsigned int NumIndices;
    unsigned int Vertex;
    unsigned int Index;
  };

  class vkRenderer;
  class vkBuffer;
  class vkCommandPool;
  class vkMesh {
  public:
    vkMesh(vkRenderer *renderer);
    vkMesh(vkRenderer *renderer, std::vector<MeshVertex> vertices, std::vector<uint32_t> indices, std::vector<vkSubMesh> submeshes);
    ~vkMesh();

    void initialize(vkCommandPool *pool);
    void cleanup();
    void render(VkCommandBuffer cmdBuf);

    void setVertices(std::vector<MeshVertex> arr) { mVertices = arr; }
    void setIndices(std::vector<uint32_t> arr)    { mIndices = arr; }
    void addSubMesh(vkSubMesh subMesh)            { mSubMeshes.push_back(subMesh); }
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    std::vector<MeshVertex> mVertices; 
    std::vector<uint32_t>   mIndices;
    std::vector<vkSubMesh>  mSubMeshes;
    vkBuffer *mVBuf = nullptr;
    vkBuffer *mIBuf = nullptr;
  };

}

#endif