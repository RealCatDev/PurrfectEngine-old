#ifndef PURRENGINE_RENDERER_HPP_
#define PURRENGINE_RENDERER_HPP_

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include "PurrfectEngine/core.hpp"
#include "PurrfectEngine/utils.hpp"
#include "PurrfectEngine/window.hpp"
#include "PurrfectEngine/mesh.hpp"
#include "PurrfectEngine/texture.hpp"

#define MAX_FRAMES_IN_FLIGHT 3

namespace PurrfectEngine {

  #define CHECK_VK(check) { auto result = (check); if(result != VK_SUCCESS) { fprintf(stderr, "Failed '%s' with \"%s\"!\n", PURR_STRINGIFY_MACRO(check), string_VkResult(result)); exit(1); } }

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

    void initialize(vkCommandPool *pool);

    void attach(vkRenderer *renderer);
  public:
    void chooseSwapSurfaceFormat(const std::vector<VkFormat>& requestedFormats, VkColorSpaceKHR requestedColorSpace);
    void chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& requestedPresentModes);
    void chooseSwapExtent(window *win);

    void setLayout(VkImageLayout layout) { mLayout = layout; }
  public:
    VkExtent2D  getExtent()           const { PURR_ASSERT(mExtent.has_value()); return mExtent.value(); }
    VkImageView getView(uint32_t idx) const { PURR_ASSERT(0 < idx < mImageViews.size()); return mImageViews[idx]; }
    VkFormat    getFormat()           const { PURR_ASSERT(mFormat.has_value()); return mFormat.value().format; }
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    std::optional<VkSurfaceFormatKHR> mFormat{};
    std::optional<VkPresentModeKHR>   mPresentMode{};
    std::optional<VkExtent2D>         mExtent{};
    std::optional<VkImageLayout>      mLayout{};
    
    std::vector<VkImage> mImages{};
    std::vector<VkImageView> mImageViews{};

    VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
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
    void setVertexBind(VkVertexInputBindingDescription *x) { mVertexBindingDescription = x; }
    void addShader(VkShaderStageFlagBits stage, vkShader *shader);
    void addDescriptor(vkDescriptorLayout *layout) { mDescriptors.push_back(layout); }
    void addPushConstant(uint32_t offset, uint32_t size, VkShaderStageFlagBits stage) { 
      VkPushConstantRange range{};
      range.stageFlags = stage;
      range.offset = offset;
      range.size = size;
      mPushConstants.push_back(range); 
    }

    void enableDepthStencil(VkBool32 testEnable, VkBool32 writeEnable, VkCompareOp compareOp, VkBool32 boundsTestEnable, VkBool32 stencilTestEnable);

    void setRenderPass(vkRenderPass *renderPass) { mRenderPass = renderPass; }

    void enableMSAA() { mMsaa = true; }
    void setCulling(VkFrontFace frontFace, VkCullModeFlagBits cullMode) { mFrontFace = frontFace; mCullMode = cullMode; }

    void initialize();

    VkPipelineLayout getLayout() const { return mLayout; }
    VkPipeline       get()       const { return mPipeline; }
  private:
    vkRenderer   *mRenderer   = nullptr;
    vkRenderPass *mRenderPass = nullptr;
  private:
    std::vector<vkShader*>                         mShaders{};
    std::vector<VkPipelineShaderStageCreateInfo>   mShaderStages{};

    std::vector<VkVertexInputAttributeDescription> mVertexAttributeDescs{};
    VkVertexInputBindingDescription               *mVertexBindingDescription = nullptr;

    std::vector<vkDescriptorLayout*>               mDescriptors{};
    std::vector<VkPushConstantRange>               mPushConstants{};

    VkPipelineDepthStencilStateCreateInfo mDepthStencil;
    bool                                  mDepthStencilEnable = false;

    VkFrontFace        mFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkCullModeFlagBits mCullMode  = VK_CULL_MODE_BACK_BIT;

    bool mMsaa = false;

    VkPipelineLayout mLayout   = VK_NULL_HANDLE;
    VkPipeline       mPipeline = VK_NULL_HANDLE;
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
    VkShaderModule mModule = VK_NULL_HANDLE;
  };

  class vkRenderPass {
    friend class vkPipeline;
    friend class vkFramebuffer;
  public:
    struct attachmentInfo {
      VkAttachmentLoadOp    loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
      VkAttachmentStoreOp   storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
      VkAttachmentLoadOp    stcLoadOp     = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      VkAttachmentStoreOp   stcStoreOp    = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      VkImageLayout         initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      VkImageLayout         finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      VkImageLayout         layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      VkFormat              format        = VK_FORMAT_UNDEFINED;
      bool                  msaaSamples   = false;
    };
  public:
    vkRenderPass(vkRenderer *renderer);
    ~vkRenderPass();

    void addAttachment(attachmentInfo info);
    void addAttachmentResolve(attachmentInfo info);

    void initialize();

    VkRenderPass get() const { return mPass; }
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    std::vector<VkImageLayout>           mAttachmentLayouts{};
    std::vector<VkImageLayout>           mAttachmentResolveLayouts{};
    std::vector<VkAttachmentDescription> mAttachmentDescs{};
    VkRenderPass mPass = VK_NULL_HANDLE;
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
    VkFramebuffer mFramebuffer = VK_NULL_HANDLE;
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
    void copyBuffer(VkBuffer buffer, VkImage image, int width, int height);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void generateMipmaps(VkImage image, VkFormat format, int width, int height, uint32_t mipLevels, VkImageLayout targetLayout);
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    VkCommandPool mPool = VK_NULL_HANDLE;
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

    VkBuffer get()  const { return mBuffer; }
    void *getData() const { return mData; }
  private:
    vkRenderer* mRenderer = nullptr;
  private:
    VkBuffer       mBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mMemory = 0;
    VkDeviceSize   mSize   = 0;
    void          *mData   = nullptr;
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

    VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;
  };

  class vkDescriptorSet {
    friend class vkDescriptorPool;
  public:
    ~vkDescriptorSet();

    void write(vkBuffer *buffer, uint32_t binding = 0, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    void write(VkImageLayout layout, VkImageView view, VkSampler sampler, uint32_t binding = 0);
    void bind(VkCommandBuffer cmdBuf, vkPipeline *pipeline, uint32_t set = 0);

    VkDescriptorSet get() const { return mSet ? mSet : VK_NULL_HANDLE; }
  private:
    vkDescriptorSet(vkRenderer *renderer, VkDescriptorSet set);

    vkRenderer *mRenderer = nullptr;
  private:
    VkDescriptorSet mSet = VK_NULL_HANDLE;
  };
  class vkDescriptorPool {
  public:
    vkDescriptorPool(vkRenderer *renderer);
    ~vkDescriptorPool();

    vkDescriptorSet *allocate(vkDescriptorLayout *layout);

    void initialize(std::vector<VkDescriptorPoolSize> sizes);
    void cleanup();

    VkDescriptorPool get() const { return mPool; }
  private:
    vkRenderer *mRenderer = nullptr;
  private:
    VkDescriptorPool mPool = VK_NULL_HANDLE;
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

    friend class vkMesh;
    friend class vkTexture;
    friend class vkCubemap;
    friend class vkSkybox;
    
    friend class ImGuiHelper;

    using SizeCallbackFn = std::function<void()>;
  public:
    vkRenderer(window *win);
    ~vkRenderer();
    
    void initialize();
    bool beginDraw();
    void endDraw(VkCommandBuffer buf);

    uint32_t frame() const { return mFrame; }

    void setSizeCallback(SizeCallbackFn cb) { mSizeCb = cb; }
    void setResizeCheck(SizeCallbackFn cb) { mCheckResize = cb; }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void createImage(int width, int height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void createCubemapImage(int width, int height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat getDepthFormat();

    VkDevice getDevice() const { return mDevice; }
  private: // Utility functions
    bool CheckLayerSupport(std::vector<const char *> layers);
    #ifdef PURR_DEBUG
      void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    #endif
    vkSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice device);
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

    VkInstance               mInstance       = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT mDebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR             mSurface        = VK_NULL_HANDLE;
    VkPhysicalDevice         mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice                 mDevice         = VK_NULL_HANDLE;

    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue  = VK_NULL_HANDLE;

    std::vector<VkSemaphore> mImageAvailableSemaphores{};
    std::vector<VkSemaphore> mRenderFinishedSemaphores{};
    std::vector<VkFence>     mInFlightFences{};

    uint32_t mFrame = 0;
    uint32_t mImageIndex = 0;

    vkSwapchain *mSwapChain = nullptr;

    VkSampleCountFlagBits mMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;  
  private: // Other
    SizeCallbackFn mSizeCb = nullptr;
    SizeCallbackFn mCheckResize = nullptr;
    window *mWindow = nullptr;
  };

}

#endif