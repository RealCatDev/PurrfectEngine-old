#ifndef PURRENGINE_RENDERER_HPP_
#define PURRENGINE_RENDERER_HPP_

#include "PurrfectEngine/core.hpp"
#include "PurrfectEngine/utils.hpp"
#include "PurrfectEngine/window.hpp"

#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 3

namespace PurrfectEngine {

  #define CHECK_VK(check) { if((check) != VK_SUCCESS) { fprintf(stderr, "Failec!"); PURR_DEBUGBREAK(); } }

  struct vkQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool operator()() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct vkSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  class vkRenderer;
  class vkSwapchain {
    friend class vkRenderer;
  public:
    vkSwapchain(vkRenderer *renderer);
    ~vkSwapchain();

    void initialize();

    void attach(vkRenderer *renderer);
  public:
    void chooseSwapSurfaceFormat(const std::vector<VkFormat>& requestedFormats, VkColorSpaceKHR requestedColorSpace);
    void chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& requestedPresentModes);
    void chooseSwapExtent(window *win);
  public:
    VkExtent2D getExtent() const { PURR_ASSERT(mExtent.has_value()); return mExtent.value(); }
    VkImageView getView(uint32_t idx) const { PURR_ASSERT(0 < idx < mImageViews.size()); return mImageViews[idx]; }
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    std::optional<VkSurfaceFormatKHR> mFormat;
    std::optional<VkPresentModeKHR> mPresentMode;
    std::optional<VkExtent2D> mExtent;
    
    std::vector<VkImage> mImages;
    std::vector<VkImageView> mImageViews;

    VkSwapchainKHR mSwapChain;
  };

  class vkShader;
  class vkRenderPass;
  class vkPipeline {
    friend class vkRenderer;
  public:
    vkPipeline(vkRenderer *renderer);
    ~vkPipeline();

    void addShader(VkShaderStageFlagBits stage, vkShader *shader);

    void setRenderPass(vkRenderPass *renderPass) { mRenderPass = renderPass; }

    void initialize();

    VkPipeline get() const { return mPipeline; }
  private:
    vkRenderer   *mRenderer   = nullptr;
    vkRenderPass *mRenderPass = nullptr;
  private:
    std::vector<VkPipelineShaderStageCreateInfo> mShaderStages{};

    VkPipelineLayout mLayout;
    VkPipeline mPipeline;
  };

  class vkShader {
    friend class vkPipeline;
  public:
    vkShader(vkRenderer *renderer);
    ~vkShader();

    void load(const char *filename);
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    VkShaderModule mModule;
  };

  class vkRenderPass {
    friend class vkPipeline;
    friend class vkFramebuffer;
  public:
    struct attachmnetInfo {
      VkAttachmentLoadOp    loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
      VkAttachmentStoreOp   storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
      VkAttachmentLoadOp    stcLoadOp     = VK_ATTACHMENT_LOAD_OP_CLEAR;
      VkAttachmentStoreOp   stcStoreOp    = VK_ATTACHMENT_STORE_OP_STORE;
      VkImageLayout         initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkImageLayout         finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      VkImageLayout         layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      VkFormat              format        = VK_FORMAT_UNDEFINED;
      VkSampleCountFlagBits samples       = VK_SAMPLE_COUNT_1_BIT;
    };
  public:
    vkRenderPass(vkRenderer *renderer);
    ~vkRenderPass();

    void addAttachment(attachmnetInfo info);

    void initialize();

    VkRenderPass get() const { return mPass; }
  private:
    vkRenderer *mRenderer;
  private:
    std::vector<VkImageLayout>           mAttachmentLayouts{};
    std::vector<VkAttachmentDescription> mAttachmentDescs{};
    VkRenderPass mPass;
  };

  class vkFramebuffer {
  public:
    vkFramebuffer(vkRenderer *renderer);
    ~vkFramebuffer();

    void addAttachment(VkImageView   attc) { mAttachments.push_back(attc); }
    void setRenderPass(vkRenderPass *pass) { mRenderPass = pass; }
    void setExtent    (VkExtent2D    extn) { mExtent = extn; }

    void initialize();

    VkFramebuffer get() const { return mFramebuffer; }
  private:
    vkRenderer   *mRenderer   = nullptr;
    vkRenderPass *mRenderPass = nullptr;
  private:
    std::vector<VkImageView> mAttachments{};
    VkExtent2D mExtent;
    VkFramebuffer mFramebuffer;
  };

  class vkCommandPool {
  public:
    vkCommandPool(vkRenderer* renderer);
    ~vkCommandPool();

    std::vector<VkCommandBuffer> allocate(uint32_t count);

    void initialize();
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    VkCommandPool mPool;
  };

  class vkRenderer {
    friend class vkSwapchain;
    friend class vkPipeline;
    friend class vkShader;
    friend class vkRenderPass;
    friend class vkFramebuffer;
    friend class vkCommandPool;
  public:
    vkRenderer(window *win);
    ~vkRenderer();
    
    void initialize();
    void beginDraw();
    void endDraw(VkCommandBuffer buf);

    uint32_t frame() const { return mFrame; }
  private: // Utility functions
    bool CheckLayerSupport(std::vector<const char *> layers);
    #ifdef PURR_DEBUG
      void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    #endif
    vkSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    VkImageView CreateImageView(VkImage image, VkFormat format);
  private: // Vulkan related functions
    void InitInstance();
    #ifdef PURR_DEBUG
      void InitDebug();
    #endif
    void CreateSurface();
    void FindQueueFamilies(VkPhysicalDevice device);
    void PickDevice();
    void CreateDevice();
    void CreateSyncObjects();

    void Cleanup();
  private: // Vulkan variables
    vkQueueFamilyIndices mQueueFamily;

    VkInstance               mInstance;
    VkDebugUtilsMessengerEXT mDebugMessenger;
    VkSurfaceKHR             mSurface;
    VkPhysicalDevice         mPhysicalDevice;
    VkDevice                 mDevice;

    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;

    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence>     mInFlightFences;

    uint32_t mFrame = 0;
    uint32_t mImageIndex;

    vkSwapchain *mSwapChain = nullptr;
  private: // Other
    window *mWindow = nullptr;
  };

}

#endif