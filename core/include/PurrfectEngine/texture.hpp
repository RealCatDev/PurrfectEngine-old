#ifndef PURRENGINE_TEXTURE_HPP_
#define PURRENGINE_TEXTURE_HPP_

#include "PurrfectEngine/core.hpp"

#include <vulkan/vulkan.h>

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

    void initialize(vkCommandPool *pool, vkDescriptorPool *descriptors, VkFormat format, VkImageLayout targetLayout, int width = -1, int height = -1, bool mipmaps = true, bool msaaSamples = false, bool descriptor = true, uint8_t *pixels = nullptr);
    void initializeHdr(vkCommandPool *pool, vkDescriptorPool *descriptors, VkFormat format, VkImageLayout targetLayout, int width = -1, int height = -1, bool mipmaps = true, bool msaaSamples = false, bool descriptor = true, float *pixels = nullptr);
    void cleanup();
    void bind(VkCommandBuffer cmdBuf, vkPipeline *pipeline, uint32_t set = 1);

    VkImage         getImage()   const { return mImage; }
    VkImageView     getView()    const { return mView; }
    VkSampler       getSampler() const { return mSampler; }
    VkDescriptorSet getSet()     const;

    static void initializeBlank(vkRenderer *renderer, vkCommandPool *pool, vkDescriptorPool *descriptors) {
      sBlank = new vkTexture(renderer);
      size_t size = sizeof(uint8_t) * 512 * 512 * 4;
      uint8_t *pixels = (uint8_t*)malloc(size);
      PURR_ASSERT(pixels, "Failed to allocate memory for pixels!");
      memset(pixels, 0xFF, size);
      sBlank->initialize(pool, descriptors, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 512, 512, false, false, true, pixels);
    }

    static vkTexture *getBlank() {
      PURR_ASSERT(sBlank, "Forgot to call static vkTexture::initializeBlank()?");
      return sBlank;
    }
  private:
    inline static vkTexture *sBlank = nullptr;
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