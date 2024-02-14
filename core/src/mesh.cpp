#include "PurrfectEngine/mesh.hpp"
#include "PurrfectEngine/renderer.hpp"

namespace PurrfectEngine {

  vkMesh::vkMesh(vkRenderer *renderer):
    mRenderer(renderer), mVertices(), mIndices()
  {}
  
  vkMesh::vkMesh(vkRenderer *renderer, std::vector<MeshVertex> vertices, std::vector<uint32_t> indices):
    mRenderer(renderer), mVertices(vertices), mIndices(indices)
  {}
  
  vkMesh::~vkMesh() {
    cleanup();
  }

  void vkMesh::initialize(vkCommandPool *pool) {
    if (mVBuf || mIBuf) cleanup();
    {
      VkDeviceSize bufferSize = sizeof(mVertices[0]) * mVertices.size();

      vkBuffer *staging = new vkBuffer(mRenderer);
      staging->initialize(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

      staging->mapMemory();
        staging->setData(mVertices.data());
      staging->unmapMemory();

      mVBuf = new vkBuffer(mRenderer);
      mVBuf->initialize(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      mVBuf->copyBuffer(pool, staging); 

      delete staging;
    }

    {
      VkDeviceSize bufferSize = sizeof(mInidces[0]) * mInidces.size();

      vkBuffer *staging = new vkBuffer(mRenderer);
      staging->initialize(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

      staging->mapMemory();
        staging->setData(mInidces.data());
      staging->unmapMemory();

      mIBuf = new vkBuffer(mRenderer);
      mIBuf->initialize(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      mIBuf->copyBuffer(pool, staging); 

      delete staging;
    }
  }

  void vkMesh::cleanup() {
    if (mVBuf) delete mVBuf;
    if (mIBuf) delete mIBuf;
  }

  void vkMesh::render(VkCommandBuffer cmdBuf) {

  }

}