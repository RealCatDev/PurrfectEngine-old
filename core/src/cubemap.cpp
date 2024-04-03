#include "PurrfectEngine/cubemap.hpp"

#include "PurrfectEngine/renderer.hpp"

namespace PurrfectEngine {

  vkCubemap::vkCubemap(vkRenderer *renderer):
    mRenderer(renderer)
  {}

  vkCubemap::~vkCubemap() {
    cleanup();
  }

  void vkCubemap::addSide(float *pixels, int width, int height) {
    if (mWidth < 0 || mHeight < 0) {
      PURR_ASSERT(width >= 0 && height >= 0);
      mWidth = width; mHeight = height;
    }
    PURR_ASSERT(mIndex < 6);
    PURR_ASSERT(width == mWidth && height == mHeight);
    PURR_ASSERT(sizeof(pixels) / sizeof(pixels[0]) == width * height * 4);
    mPixels[mIndex++] = pixels;
  }

  void vkCubemap::initialize(vkCommandPool *pool, vkDescriptorPool *descriptors, VkFormat format, VkImageLayout targetLayout) {
    vkBuffer *stagingBuffer = nullptr;
    if (mIndex > 0) { PURR_ASSERT(mIndex == 6);
      VkDeviceSize imageSize = mWidth * mHeight * 4;
      VkDeviceSize imagesSize = imageSize * 6 * sizeof(float);
      stagingBuffer = new vkBuffer(mRenderer);
      stagingBuffer->initialize(imagesSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
      
      void *data = malloc(imagesSize);
      for (uint8_t i = 0; i < 6; ++i) memcpy(data + (imageSize * i), mPixels[i], imageSize);

      stagingBuffer->mapMemory();
        stagingBuffer->setData(data);
      stagingBuffer->unmapMemory();
    }
    
    mRenderer->createCubemapImage(mWidth, mHeight, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory);

    pool->transitionImageLayout(mImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
    if (stagingBuffer) {
      pool->copyBuffer(stagingBuffer->get(), mImage, mWidth, mHeight);
      delete stagingBuffer;
    }
    pool->transitionImageLayout(mImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, targetLayout, 1);
  
    mView = mRenderer->createImageView(mImage, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

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
      samplerInfo.minLod = 0.0f;
      samplerInfo.maxLod = static_cast<float>(1);
      samplerInfo.mipLodBias = 0.0f;

      CHECK_VK(vkCreateSampler(mRenderer->mDevice, &samplerInfo, nullptr, &mSampler));
    }

    mSet = descriptors->allocate(getTextureLayout());
    mSet->write(targetLayout, mView, mSampler);
  }

  void vkCubemap::cleanup() {
    if (mIndex > 0) for (uint8_t i = (mIndex = 0); i < 6; ++i) free(mPixels[i]);
    delete mSet;
  }

}