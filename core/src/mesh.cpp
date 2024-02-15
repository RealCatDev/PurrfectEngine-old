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
      mVBuf->copy(pool, staging); 

      delete staging;
    }

    {
      VkDeviceSize bufferSize = sizeof(mIndices[0]) * mIndices.size();

      vkBuffer *staging = new vkBuffer(mRenderer);
      staging->initialize(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

      staging->mapMemory();
        staging->setData(mIndices.data());
      staging->unmapMemory();

      mIBuf = new vkBuffer(mRenderer);
      mIBuf->initialize(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
      mIBuf->copy(pool, staging); 

      delete staging;
    }
  }

  void vkMesh::cleanup() {
    if (mVBuf) delete mVBuf;
    if (mIBuf) delete mIBuf;
  }

  void vkMesh::render(VkCommandBuffer cmdBuf) {
    VkDeviceSize offsets[] = {0};
    auto vbuf = mVBuf->get();
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, &vbuf, offsets);

    vkCmdBindIndexBuffer(cmdBuf, mIBuf->get(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmdBuf, static_cast<uint32_t>(mIndices.size()), 1, 0, 0, 0);
  }

}