#ifndef PURRENGINE_TEXTURE_HPP_
#define PURRENGINE_TEXTURE_HPP_

#include "PurrfectEngine/core.hpp"

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace PurrfectEngine {

  class vkRenderer;
  class vkDescriptorLayout;
  vkDescriptorLayout *getTextureLayout(vkRenderer *renderer = nullptr);
  void destroyTextureLayout();

  class vkCommandPool;
  class vkPipeline;
  class vkDescriptorPool;
  class vkDescriptorSet;
  class vkTexture {
  public:
    vkTexture(vkRenderer *renderer);
    vkTexture(vkRenderer *renderer, const char *filename);
    ~vkTexture();

    void setFilename(const char *filename) { mFilename = (char*)filename; }

    void initialize(vkCommandPool *pool, vkDescriptorPool *descriptors, VkFormat format, VkImageLayout targetLayout, int width = -1, int height = -1, bool mipmaps = true, bool msaaSamples = false, bool descriptor = true);
    void cleanup();
    void bind(VkCommandBuffer cmdBuf, vkPipeline *pipeline, uint32_t set = 1);

    VkImage         getImage() const { return mImage; }
    VkImageView     getView()  const { return mView; }
    VkDescriptorSet getSet()   const;
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    VkImage          mImage;
    VkDeviceMemory   mImageMemory;
    VkImageView      mView;
    VkSampler        mSampler;
    uint32_t         mMipLevels;

    vkDescriptorSet *mSet = nullptr;
    bool             mSetB = false;

    char *mFilename = nullptr;
  };

}

#endif