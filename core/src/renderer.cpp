#include "PurrfectEngine/renderer.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace PurrfectEngine {

  static VKAPI_ATTR VkBool32 VKAPI_CALL sDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    if (pCallbackData->pMessage) fprintf(stderr, "[Vulkan] (Validation): %s\n", pCallbackData->pMessage);
    
    return VK_FALSE;
  }

  vkSwapchain::vkSwapchain(vkRenderer *renderer):
    mRenderer(renderer)
  {}

  vkSwapchain::~vkSwapchain() {
    if (!mSwapChain) return;
    vkDestroySwapchainKHR(mRenderer->mDevice, mSwapChain, nullptr);

    for (auto view : mImageViews) {
      vkDestroyImageView(mRenderer->mDevice, view, nullptr);
    }
  }

  void vkSwapchain::initialize() {
    auto swapChainSupport = mRenderer->QuerySwapChainSupport(mRenderer->mPhysicalDevice);

    PURR_ASSERT(mFormat.has_value());
    PURR_ASSERT(mPresentMode.has_value());
    PURR_ASSERT(mExtent.has_value());

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
      imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mRenderer->mSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = mFormat.value().format;
    createInfo.imageColorSpace = mFormat.value().colorSpace;
    createInfo.imageExtent = mExtent.value();
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto queueFamilies = mRenderer->mQueueFamily;

    const uint32_t queueFamilyIndices[2] = {queueFamilies.graphicsFamily.value(), queueFamilies.presentFamily.value()};
    if (queueFamilies.graphicsFamily != queueFamilies.presentFamily) {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;
      createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = mPresentMode.value();
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    CHECK_VK(vkCreateSwapchainKHR(mRenderer->mDevice, &createInfo, nullptr, &mSwapChain));

    {
      vkGetSwapchainImagesKHR(mRenderer->mDevice, mSwapChain, &imageCount, nullptr);
      mImages.resize(imageCount);
      vkGetSwapchainImagesKHR(mRenderer->mDevice, mSwapChain, &imageCount, mImages.data());
    }

    {
      mImageViews.resize(imageCount);
      uint32_t i = 0;
      for (auto &img : mImages) mImageViews[i++] = mRenderer->CreateImageView(img, mFormat.value().format);
    }
  }

  void vkSwapchain::attach(vkRenderer *renderer) {
    renderer->mSwapChain = this;
  }

  void vkSwapchain::chooseSwapSurfaceFormat(const std::vector<VkFormat>& requestedFormats, VkColorSpaceKHR requestedColorSpace) {
    auto support = mRenderer->QuerySwapChainSupport(mRenderer->mPhysicalDevice);
    auto availableFormats = support.formats;
    for (const auto& availableFormat : availableFormats) 
      if ((std::find(requestedFormats.begin(), requestedFormats.end(), availableFormat.format) != requestedFormats.end()) && availableFormat.colorSpace == requestedColorSpace) mFormat = availableFormat;
    mFormat = availableFormats[0];
  }

  void vkSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& requestedPresentModes) {
    auto support = mRenderer->QuerySwapChainSupport(mRenderer->mPhysicalDevice);
    auto availablePresentModes = support.presentModes;
    for (const auto &mode : requestedPresentModes) 
      if (std::find(availablePresentModes.begin(), availablePresentModes.end(), mode) != availablePresentModes.end()) mPresentMode = mode;
    mPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  }

  void vkSwapchain::chooseSwapExtent(window *win) {
    auto support = mRenderer->QuerySwapChainSupport(mRenderer->mPhysicalDevice);
    auto capabilities = support.capabilities;
    if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
      int width, height;
      glfwGetFramebufferSize(win->get(), &width, &height);

      VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
      };

      actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

      mExtent = actualExtent;
    } else mExtent = capabilities.currentExtent;
  }

  vkPipeline::vkPipeline(vkRenderer *renderer):
    mRenderer(renderer)
  {}

  vkPipeline::~vkPipeline() {
    if (!mLayout || !mPipeline) return;
    vkDestroyPipelineLayout(mRenderer->mDevice, mLayout, nullptr);
    vkDestroyPipeline      (mRenderer->mDevice, mPipeline, nullptr);
  }

  void vkPipeline::addShader(VkShaderStageFlagBits stage, vkShader *shader) {
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = stage;
    stageInfo.module = shader->mModule;
    stageInfo.pName = "main";
    mShaderStages.push_back(stageInfo);
    mShaders.push_back(shader);
  }

  void vkPipeline::initialize() {
    PURR_ASSERT(mRenderPass);

    std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
  
    CHECK_VK(vkCreatePipelineLayout(mRenderer->mDevice, &pipelineLayoutInfo, nullptr, &mLayout));

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(mShaderStages.size());
    pipelineInfo.pStages = mShaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // TODO
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = mLayout;
    pipelineInfo.renderPass = mRenderPass->mPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    CHECK_VK(vkCreateGraphicsPipelines(mRenderer->mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline));
    for (auto shdr : mShaders) delete shdr;
  }

  vkShader::vkShader(vkRenderer *renderer):
    mRenderer(renderer)
  {}

  vkShader::~vkShader() {
    vkDestroyShaderModule(mRenderer->mDevice, mModule, nullptr);
  }

  void vkShader::load(const char *filename) {
    auto data = IO::ReadFile(filename);
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = data.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());
    CHECK_VK(vkCreateShaderModule(mRenderer->mDevice, &createInfo, nullptr, &mModule));
  }

  vkRenderPass::vkRenderPass(vkRenderer *renderer):
    mRenderer(renderer)
  {}
  
  vkRenderPass::~vkRenderPass() {
    if (!mPass) return;
    
    vkDestroyRenderPass(mRenderer->mDevice, mPass, nullptr);
  }

  void vkRenderPass::addAttachment(attachmnetInfo info) {
    VkAttachmentDescription desc{};
    desc.loadOp         = info.loadOp;
    desc.storeOp        = info.storeOp;
    desc.stencilLoadOp  = info.stcLoadOp;
    desc.stencilStoreOp = info.stcStoreOp;
    desc.initialLayout  = info.initialLayout;
    desc.finalLayout    = info.finalLayout;
    desc.format         = info.format;
    desc.samples        = info.samples;
    mAttachmentDescs.push_back(desc);
    mAttachmentLayouts.push_back(info.layout);
  }

  void vkRenderPass::initialize() {
    uint32_t depthRefIdx = -1;
    std::vector<VkAttachmentReference> refs{};
    VkAttachmentReference *depthRef{};

    uint32_t i = 0;
    for (const auto &desc : mAttachmentDescs) {
      VkAttachmentReference ref{};
      ref.attachment = i;
      ref.layout = mAttachmentLayouts[i];
      if (mAttachmentLayouts[i++] == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
        depthRef = &ref;
      else refs.push_back(ref);
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(refs.size());
    subpass.pColorAttachments = refs.data();
    if (depthRefIdx >= 0) subpass.pDepthStencilAttachment = depthRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(mAttachmentDescs.size());
    renderPassInfo.pAttachments = mAttachmentDescs.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    CHECK_VK(vkCreateRenderPass(mRenderer->mDevice, &renderPassInfo, nullptr, &mPass));
  }

  vkFramebuffer::vkFramebuffer(vkRenderer *renderer):
    mRenderer(renderer) 
  {}
  
  vkFramebuffer::~vkFramebuffer() {
    if (!mFramebuffer) return;
    vkDestroyFramebuffer(mRenderer->mDevice, mFramebuffer, nullptr);
  }

  void vkFramebuffer::initialize() {
    PURR_ASSERT(mRenderPass);

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass      = mRenderPass->mPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(mAttachments.size());
    framebufferInfo.pAttachments    = mAttachments.data();
    framebufferInfo.width           = mExtent.width;
    framebufferInfo.height          = mExtent.height;
    framebufferInfo.layers          = 1;

    CHECK_VK(vkCreateFramebuffer(mRenderer->mDevice, &framebufferInfo, nullptr, &mFramebuffer));
  }

  vkCommandPool::vkCommandPool(vkRenderer* renderer):
    mRenderer(renderer)
  {}

  vkCommandPool::~vkCommandPool() {
    vkDestroyCommandPool(mRenderer->mDevice, mPool, nullptr);
  }

  std::vector<VkCommandBuffer> vkCommandPool::allocate(uint32_t count) {
    PURR_ASSERT(mPool);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = count;

    std::vector<VkCommandBuffer> buffers{};
    buffers.resize(3);
    CHECK_VK(vkAllocateCommandBuffers(mRenderer->mDevice, &allocInfo, buffers.data()));
    return buffers;
  }

  VkCommandBuffer vkCommandPool::allocate() {
    PURR_ASSERT(mPool);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer buffer;
    CHECK_VK(vkAllocateCommandBuffers(mRenderer->mDevice, &allocInfo, &buffer));
    return buffer;
  }

  void vkCommandPool::initialize() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = mRenderer->mQueueFamily.graphicsFamily.value();
    CHECK_VK(vkCreateCommandPool(mRenderer->mDevice, &poolInfo, nullptr, &mPool));
  }

  VkCommandBuffer vkCommandPool::beginSingleTimeCommands() {
    VkCommandBuffer commandBuffer = allocate();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
  }

  void vkCommandPool::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(mRenderer->mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mRenderer->mGraphicsQueue);

    vkFreeCommandBuffers(mRenderer->mDevice, mPool, 1, &commandBuffer);
  }

  void vkCommandPool::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
  }

  vkBuffer::vkBuffer(vkRenderer* renderer):
    mRenderer(renderer)
  {}

  vkBuffer::~vkBuffer() {
    cleanup();
  }

  void vkBuffer::initialize(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    mInitialized = true;
    mSize = size;
    mRenderer->CreateBuffer(size, usage, properties, mBuffer, mMemory);
  }

  void vkBuffer::cleanup() {
    PURR_ASSERT(mInitialized, "Buffer was not initialized! Did you forgot to call vkBuffer::initialize(VkDeviceSize)?");
    mInitialized = false;
    mMapped = false;
    vkDestroyBuffer(mRenderer->mDevice, mBuffer, nullptr);
    vkFreeMemory(mRenderer->mDevice, mMemory, nullptr);  
  }

  void vkBuffer::mapMemory() {
    PURR_ASSERT(!mMapped, "Buffer was already mapped!");

    vkMapMemory(mRenderer->mDevice, mMemory, 0, mSize, 0, &mData);
  }

  void vkBuffer::setData(void *data) {
    PURR_ASSERT(mMapped, "Buffer was not mapped! Did you forgor to call vkBuffer::unmapMemory()?");

    memcpy(mData, data, (size_t)mSize);
  }

  void vkBuffer::unmapMemory() {
    PURR_ASSERT(mMapped, "Buffer was not mapped! Did you forgor to call vkBuffer::unmapMemory()?");

    vkUnmapMemory(mRenderer->mDevice, mMemory);
  }

  void vkBuffer::copy(vkCommandPool *pool, vkBuffer *src) {
    PURR_ASSERT(mSize == src->mSize, "vkBuffer::copy(vkCommandPool*, vkBuffer*), sizes are not the same!");
    VkCommandBuffer commandBuffer = pool->beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = src->mSize;
    vkCmdCopyBuffer(commandBuffer, src->mBuffer, mBuffer, 1, &copyRegion);
    pool->endSingleTimeCommands(commandBuffer);
  }

  vkRenderer::vkRenderer(window *win):
    mWindow(win)
  {}

  vkRenderer::~vkRenderer() {
    Cleanup();
  }

  void vkRenderer::initialize() {
    InitInstance();
    #ifdef PURR_DEBUG
      InitDebug();
    #endif
    CreateSurface();
    PickDevice();
    CreateDevice();
    CreateSyncObjects();
  }

  bool vkRenderer::beginDraw() {
    PURR_ASSERT(mSizeCb);
    PURR_ASSERT(mSwapChain, "Failed! Swapchain is not present, did you forgor to call `PurrfectEngine::vkSwapchain::attach(PurrfectEngine::vkRenderer*)`?");

    vkWaitForFences(mDevice, 1, &mInFlightFences[mFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain->mSwapChain, UINT64_MAX, mImageAvailableSemaphores[mFrame], VK_NULL_HANDLE, &mImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      Resize();
      return false;
    } else PURR_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR, "Failed to acquire swapchain image!");
    vkResetFences(mDevice, 1, &mInFlightFences[mFrame]);
    return true;
  }

  void vkRenderer::endDraw(VkCommandBuffer buf) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buf;

    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    CHECK_VK(vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mFrame]));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &(mSwapChain->mSwapChain);
    presentInfo.pImageIndices = &mImageIndex;
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
    // TODO(CatDev): Maybe add an option for user to set whether they want to resize or not?
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
      Resize();
    } else PURR_ASSERT(result == VK_SUCCESS, "Failed to present swapchain image!");

    mFrame = (++mFrame % 3);

    vkDeviceWaitIdle(mDevice);
  }

  bool vkRenderer::CheckLayerSupport(std::vector<const char *> layers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : layers) {
      bool layerFound = false;

      for (const auto& layerProperties : availableLayers) {
        if (strcmp(layerName, layerProperties.layerName) == 0) {
          layerFound = true;
          break;
        }
      }

      if (!layerFound) {
        return false;
      }
    }

    return true;
  }

  #ifdef PURR_DEBUG
    void vkRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
      createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      createInfo.pfnUserCallback = sDebugCallback;
    }
  #endif

  vkSwapChainSupportDetails vkRenderer::QuerySwapChainSupport(VkPhysicalDevice device) {
    vkSwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);

    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());
    }

    return details;
  }

  VkImageView vkRenderer::CreateImageView(VkImage image, VkFormat format) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    CHECK_VK(vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView));
    return imageView;
  }

  uint32_t vkRenderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;

    PURR_ASSERT("Failed to find suitable memory type!");
    return 0;
  }

  void vkRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    CHECK_VK(vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    CHECK_VK(vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory));

    vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);
  }

  void vkRenderer::InitInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "meow";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "PurrfectEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = mWindow->getVkExtensions();
    auto layers = std::vector<const char*>();

    #ifdef PURR_DEBUG
      layers.push_back("VK_LAYER_KHRONOS_validation");
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

      printf("Available extensions:\n");
      {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, deviceExtensions.data());
        for (const auto &ext : deviceExtensions) printf("\t- %s\n", ext);
      }

      printf("Requested extensions:\n");
      for (const auto &ext : extensions) printf("\t- %s\n", ext);



      VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
      PopulateDebugMessengerCreateInfo(debugCreateInfo);
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    #endif

    if (!CheckLayerSupport(layers)) {
      fprintf(stderr, "Failed to initialize vulkan instance! Error: not every requested layer is supported!");
      exit(1);
    }

    createInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount       = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames     = layers.data();

    CHECK_VK(vkCreateInstance(&createInfo, nullptr, &mInstance));
  }

  #ifdef PURR_DEBUG
    void vkRenderer::InitDebug() {
      VkDebugUtilsMessengerCreateInfoEXT createInfo;
      PopulateDebugMessengerCreateInfo(createInfo);

      auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
      PURR_ASSERT(func);
      CHECK_VK(func(mInstance, &createInfo, nullptr, &mDebugMessenger));
    }
  #endif

  void vkRenderer::CreateSurface() {
    CHECK_VK(glfwCreateWindowSurface(mInstance, mWindow->get(), nullptr, &mSurface));
  }

  void vkRenderer::FindQueueFamilies(VkPhysicalDevice device) {
    vkQueueFamilyIndices indices = {};
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphicsFamily = i;
      }
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);
      if (presentSupport) {
        indices.presentFamily = i;
      }

      if (indices()) break;

      i++;
    }

    mQueueFamily = indices;
  }

  void vkRenderer::PickDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
    if (deviceCount <= 0) {
      fprintf(stderr, "Failed to pick physical device! No GPUs found.\n");
      exit(1);
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

    std::unordered_map<VkPhysicalDeviceType, int> deviceTypeToScoreTbl = {
      { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, 10 },
      { VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, 5 },
      { VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, 3 },
      { VK_PHYSICAL_DEVICE_TYPE_CPU, 1 }
    };

    int curScore = 0;
    std::unordered_map<int, VkPhysicalDevice> devicesMap{};
    for (const auto &device : devices) {
      VkPhysicalDeviceProperties deviceProperties;
      VkPhysicalDeviceFeatures deviceFeatures;
      vkGetPhysicalDeviceProperties(device, &deviceProperties);
      vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

      vkSwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
      bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

      if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
          !deviceFeatures.geometryShader ||
          !(FindQueueFamilies(device), mQueueFamily()) ||
          !swapChainAdequate) continue; // not suitable

      int score = 0;

      score += deviceTypeToScoreTbl[deviceProperties.deviceType];
      score += deviceProperties.limits.maxImageDimension2D;

      if (score > curScore) curScore = score;
      devicesMap[curScore] = device;
    }

    if (devicesMap.size() <= 0) {
      fprintf(stderr, "Failed to pick physical device! No suitable GPUs found.\n");
      exit(1);
    }

    mPhysicalDevice = devicesMap[curScore];
    FindQueueFamilies(mPhysicalDevice);
  }

  void vkRenderer::CreateDevice() {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = mQueueFamily.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pEnabledFeatures = &deviceFeatures;

    std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    std::vector<const char*> layers{};

    #ifdef PURR_DEBUG
      layers.push_back("VK_LAYER_KHRONOS_validation");
    #endif

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
  
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {mQueueFamily.graphicsFamily.value(), mQueueFamily.presentFamily.value()};

    {
      float queuePriority = 1.0f;
      for (uint32_t queueFamily : uniqueQueueFamilies) {
          VkDeviceQueueCreateInfo queueCreateInfo{};
          queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
          queueCreateInfo.queueFamilyIndex = queueFamily;
          queueCreateInfo.queueCount = 1;
          queueCreateInfo.pQueuePriorities = &queuePriority;
          queueCreateInfos.push_back(queueCreateInfo);
      }
    }

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    CHECK_VK(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice));

    vkGetDeviceQueue(mDevice, mQueueFamily.graphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, mQueueFamily.presentFamily.value(), 0, &mPresentQueue);
  }

  void vkRenderer::CreateSyncObjects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  
    mImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    mInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      CHECK_VK(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]));
      CHECK_VK(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]));
      CHECK_VK(vkCreateFence    (mDevice, &fenceInfo,     nullptr, &mInFlightFences[i]          ));
    }
  }

  void vkRenderer::Resize() {
    int w = 0, h = 0;
    mWindow->getSize(&w, &h);
    while (w == 0 || h == 0) {
      mWindow->getSize(&w, &h);
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice);

    mSizeCb();
  }

  void vkRenderer::Cleanup() {
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
      vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
      vkDestroyFence    (mDevice, mInFlightFences[i],           nullptr);
    }

    vkDestroyDevice(mDevice, nullptr);

    #ifdef PURR_DEBUG
      auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
      if (func != nullptr) {
        func(mInstance, mDebugMessenger, nullptr);
      }
    #endif

    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

    vkDestroyInstance(mInstance, nullptr);
  }

}