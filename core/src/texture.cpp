#include "PurrfectEngine/texture.hpp"
#include "PurrfectEngine/renderer.hpp"

#include <stb_image.h>

namespace PurrfectEngine {

  static vkDescriptorLayout *sTextureLayout = nullptr;

  vkDescriptorLayout *getTextureLayout(vkRenderer *renderer) { 
    if (!sTextureLayout) PURR_ASSERT(renderer);
    if (!sTextureLayout) {
      sTextureLayout = new vkDescriptorLayout(renderer);
      sTextureLayout->addBinding({
        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr
      });
      sTextureLayout->initialize();
    }
    return sTextureLayout;
  }

  void destroyTextureLayout() {
    delete sTextureLayout;
  }

  vkTexture::vkTexture(vkRenderer *renderer):
    mRenderer(renderer)
  {}

  vkTexture::vkTexture(vkRenderer *renderer, const char *filename):
    mRenderer(renderer), mFilename((char*)filename)
  {}

  vkTexture::~vkTexture() {
    cleanup();
  }

  void vkTexture::initialize(vkCommandPool *pool, vkDescriptorPool *descriptors, VkFormat format) {
    PURR_ASSERT(mFilename, "Failed! File name was not set, did you forgor to call vkTexture::setFilename(const char*)?");

    // Image
    {
      int texWidth, texHeight, texChannels;
      stbi_uc* pixels = stbi_load(mFilename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
      VkDeviceSize imageSize = texWidth * texHeight * 4;

      PURR_ASSERT(pixels, "Failed to load texture!");

      vkBuffer *stagingBuffer = new vkBuffer(mRenderer);
      stagingBuffer->initialize(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      
      stagingBuffer->mapMemory();
      stagingBuffer->setData(pixels);
      stagingBuffer->unmapMemory();

      stbi_image_free(pixels);

      mRenderer->CreateImage(texWidth, texHeight, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory);

      pool->transitionImageLayout(mImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
      pool->copyBuffer           (stagingBuffer->get(), mImage, texWidth, texHeight);
      pool->transitionImageLayout(mImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      delete stagingBuffer;
    }

    // Image view
    mView = mRenderer->CreateImageView(mImage, format);

    // Sampler
    {
      VkSamplerCreateInfo samplerInfo{};
      samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      samplerInfo.magFilter = VK_FILTER_LINEAR;
      samplerInfo.minFilter = VK_FILTER_LINEAR;
      samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      samplerInfo.anisotropyEnable = VK_FALSE;
      samplerInfo.maxAnisotropy = 1.0f;
      samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      samplerInfo.unnormalizedCoordinates = VK_FALSE;
      samplerInfo.compareEnable = VK_FALSE;
      samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
      samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      samplerInfo.mipLodBias = 0.0f;
      samplerInfo.minLod = 0.0f;
      samplerInfo.maxLod = 0.0f;

      CHECK_VK(vkCreateSampler(mRenderer->mDevice, &samplerInfo, nullptr, &mSampler));
    }

    // Descriptor set
    {
      mSet = descriptors->allocate(getTextureLayout());
      mSet->write(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mView, mSampler);
    }
  }

  void vkTexture::cleanup() {
    if (mSet) delete mSet;
    vkDestroySampler(mRenderer->mDevice, mSampler, nullptr);
    vkDestroyImageView(mRenderer->mDevice, mView, nullptr);
    vkDestroyImage(mRenderer->mDevice, mImage, nullptr);
    vkFreeMemory(mRenderer->mDevice, mImageMemory, nullptr);
  }

  void vkTexture::bind(VkCommandBuffer cmdBuf, vkPipeline *pipeline, uint32_t set) {
    PURR_ASSERT(mSet, "Failed to bind texture! Texture not initialized, did you forgor to call void vkTexture::initialize(vkCommandPool *, vkDescriptorPool *, VkFormat)?");

    mSet->bind(cmdBuf, pipeline, set);
  }

}