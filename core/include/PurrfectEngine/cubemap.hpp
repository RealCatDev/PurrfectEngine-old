#ifndef   PURRENGINE_CUBEMAP_HPP_
#define   PURRENGINE_CUBEMAP_HPP_

#include "PurrfectEngine/texture.hpp"

namespace PurrfectEngine {

  class vkCubemap {
  public:
    vkCubemap(vkRenderer *renderer);
    ~vkCubemap();

    void addSide(float *pixels, int width, int height);

    void initialize(vkCommandPool *pool, vkDescriptorPool *descriptors, VkFormat format, VkImageLayout targetLayout);
    void cleanup();

    vkDescriptorSet *getSet() const { return mSet; }
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    int mWidth = -1, mHeight = -1;

    vkDescriptorSet     *mSet = nullptr;

    std::array<float*,6> mPixels{};
    uint8_t              mIndex = 0;

    VkDeviceMemory mImageMemory;
    VkImage        mImage;
    VkImageView    mView;
    VkSampler      mSampler;
  };

}

#endif // PURRENGINE_CUBEMAP_HPP_