#ifndef PURRENGINE_RENDERER_HPP_
#define PURRENGINE_RENDERER_HPP_

#include "PurrfectEngine/core.hpp"
#include "PurrfectEngine/utils.hpp"
#include "PurrfectEngine/window.hpp"
#include "PurrfectEngine/mesh.hpp"

#include <vulkan/vulkan.h>

#define MAX_FRAMES_IN_FLIGHT 3

namespace PurrfectEngine {

  #define CHECK_VK(check) { if((check) != VK_SUCCESS) { fprintf(stderr, "Failed '%s'!", PURR_STRINGIFY_MACRO(check)); PURR_DEBUGBREAK(); } }

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
  class vkDescriptorLayout;
  class vkPipeline {
    friend class vkRenderer;
    friend class vkDescriptorSet;
  public:
    vkPipeline(vkRenderer *renderer);
    ~vkPipeline();

    void setVertexAttrs(std::vector<VkVertexInputAttributeDescription> x) { mVertexAttributeDescs = x; }
    void setVertexBind(VkVertexInputBindingDescription x) { mVertexBindingDescription = &x; }
    void addShader(VkShaderStageFlagBits stage, vkShader *shader);
    void addDescriptor(vkDescriptorLayout *layout) { mDescriptors.push_back(layout); }

    void setRenderPass(vkRenderPass *renderPass) { mRenderPass = renderPass; }

    void initialize();

    VkPipeline get() const { return mPipeline; }
  private:
    vkRenderer   *mRenderer   = nullptr;
    vkRenderPass *mRenderPass = nullptr;
  private:
    std::vector<vkShader*>                         mShaders{};
    std::vector<VkPipelineShaderStageCreateInfo>   mShaderStages{};

    std::vector<VkVertexInputAttributeDescription> mVertexAttributeDescs{};
    VkVertexInputBindingDescription               *mVertexBindingDescription = nullptr;

    std::vector<vkDescriptorLayout*>               mDescriptors{};

    VkPipelineLayout mLayout;
    VkPipeline       mPipeline;
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
    VkCommandBuffer              allocate();

    void initialize();

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    VkCommandPool mPool;
  };

  class vkBuffer {
    friend class vkDescriptorSet;
  public:
    vkBuffer(vkRenderer* renderer);
    ~vkBuffer();

    void initialize(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void cleanup();

    void mapMemory();
    void setData(void *data);
    void unmapMemory();

    void copy(vkCommandPool *pool, vkBuffer *src);

    VkBuffer get() const { return mBuffer; }
  private:
    vkRenderer* mRenderer = nullptr;
  private:
    VkBuffer       mBuffer;
    VkDeviceMemory mMemory;
    VkDeviceSize   mSize;
    void          *mData;
  private:
    bool mInitialized = false;
    bool mMapped      = false;
  };

  class vkDescriptorLayout {
  public:
    vkDescriptorLayout(vkRenderer *renderer);
    ~vkDescriptorLayout();

    void addBinding(VkDescriptorSetLayoutBinding binding) { mBindings.push_back(binding); }

    void initialize();
    void cleanup();

    VkDescriptorSetLayout get() const { return mLayout; }
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    std::vector<VkDescriptorSetLayoutBinding> mBindings{};

    VkDescriptorSetLayout mLayout;
  };

  class vkDescriptorSet {
    friend class vkDescriptorPool;
  public:
    ~vkDescriptorSet();

    void write(vkBuffer *buffer, uint32_t binding = 0);
    void bind(VkCommandBuffer cmdBuf, vkPipeline *pipeline, uint32_t set = 0);
  private:
    vkDescriptorSet(vkRenderer *renderer, VkDescriptorSet set);

    vkRenderer *mRenderer = nullptr;
  private:
    VkDescriptorSet mSet;
  };

  class vkDescriptorPool {
  public:
    vkDescriptorPool(vkRenderer *renderer);
    ~vkDescriptorPool();

    vkDescriptorSet *allocate(vkDescriptorLayout *layout);

    void initialize(std::vector<VkDescriptorPoolSize> sizes);
    void cleanup();
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    VkDescriptorPool mPool;
  };

  class vkRenderer {
    friend class vkSwapchain;
    friend class vkPipeline;
    friend class vkShader;
    friend class vkRenderPass;
    friend class vkFramebuffer;
    friend class vkCommandPool;
    friend class vkBuffer;
    friend class vkDescriptorLayout;
    friend class vkDescriptorSet;
    friend class vkDescriptorPool;

    using SizeCallbackFn = std::function<void()>;
  public:
    vkRenderer(window *win);
    ~vkRenderer();
    
    void initialize();
    bool beginDraw();
    void endDraw(VkCommandBuffer buf);

    uint32_t frame() const { return mFrame; }

    void setSizeCallback(SizeCallbackFn cb) { mSizeCb = cb; }
  private: // Utility functions
    bool CheckLayerSupport(std::vector<const char *> layers);
    #ifdef PURR_DEBUG
      void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    #endif
    vkSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    VkImageView CreateImageView(VkImage image, VkFormat format);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
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

    void Resize();

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
    SizeCallbackFn mSizeCb = nullptr;
    window *mWindow = nullptr;
  };

}

#endif