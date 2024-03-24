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

  void vkTexture::initialize(vkCommandPool *pool, vkDescriptorPool *descriptors, VkFormat format, VkImageLayout targetLayout, int width, int height, bool mipmaps, bool msaaSamples, bool descriptor) {
    bool depth = targetLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Image
    int texWidth, texHeight;
    vkBuffer *stagingBuffer = nullptr;
    {
      if (mFilename) {
        int texChannels;
        stbi_uc* pixels = stbi_load(mFilename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        PURR_ASSERT(pixels, "Failed to load texture!");

        stagingBuffer = new vkBuffer(mRenderer);
        stagingBuffer->initialize(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        stagingBuffer->mapMemory();
        stagingBuffer->setData(pixels);
        stagingBuffer->unmapMemory();

        stbi_image_free(pixels);
      } else { PURR_ASSERT(width != -1 && height != -1, "(Texture) Width and height have to be set when not using filename!"); texWidth = width; texHeight = height; }

      mMipLevels = mipmaps ? (static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1) : 1;

      mRenderer->createImage(texWidth, texHeight, mMipLevels, msaaSamples ? mRenderer->mMsaaSamples : VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | (depth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory);

      if (mipmaps) pool->transitionImageLayout(mImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mMipLevels);
      if (mFilename) {
        pool->copyBuffer           (stagingBuffer->get(), mImage, texWidth, texHeight);
      }

      if (stagingBuffer) delete stagingBuffer;
    }
    if (mipmaps) {
      pool->generateMipmaps(mImage, format, texWidth, texHeight, mMipLevels, targetLayout);
    } //else pool->transitionImageLayout(mImage, format, VK_IMAGE_LAYOUT_UNDEFINED, targetLayout, 1);

    // Image view
    mView = mRenderer->createImageView(mImage, format, depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT, mMipLevels);

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
      samplerInfo.minLod = 0.0f;
      samplerInfo.maxLod = static_cast<float>(mMipLevels);
      samplerInfo.mipLodBias = 0.0f;

      CHECK_VK(vkCreateSampler(mRenderer->mDevice, &samplerInfo, nullptr, &mSampler));
    }

    // Descriptor set
    if (descriptor) {
      mSet = descriptors->allocate(getTextureLayout());
      mSet->write(targetLayout, mView, mSampler);
      mSetB = true;
    }
  }

  void vkTexture::cleanup() {
    if (mSetB) { mSetB = false; delete mSet; }
    vkDestroySampler(mRenderer->mDevice, mSampler, nullptr);
    vkDestroyImageView(mRenderer->mDevice, mView, nullptr);
    vkDestroyImage(mRenderer->mDevice, mImage, nullptr);
    vkFreeMemory(mRenderer->mDevice, mImageMemory, nullptr);
  }

  void vkTexture::bind(VkCommandBuffer cmdBuf, vkPipeline *pipeline, uint32_t set) {
    PURR_ASSERT(mSet, "Failed to bind texture! Texture not initialized, did you forgor to call void vkTexture::initialize(vkCommandPool *, vkDescriptorPool *, VkFormat)?");

    mSet->bind(cmdBuf, pipeline, set);
  }

  VkDescriptorSet vkTexture::getSet() const { 
    return mSet ? mSet->get() : VK_NULL_HANDLE;
  }

}