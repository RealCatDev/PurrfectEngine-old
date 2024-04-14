#include "PurrfectEngine/renderer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

const char **getWindowExtensions(uint32_t *count) {
  return glfwGetRequiredInstanceExtensions(count);
}

bool vkCheckLayerSupport(const char **inLayers, uint32_t inLayerCount) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, NULL);

  VkLayerProperties *layers = malloc(sizeof(VkLayerProperties)*layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, layers);

  for (size_t i = 0; i < inLayerCount; ++i) {
    const char *name = inLayers[i];
    bool found = false;

    for (size_t j = 0; j < layerCount; ++j) {
      VkLayerProperties layer = layers[i];
      if (strcmp(name, layer.layerName) == 0) { found = true; break; }
    }

    if (!found) return false;
  }

  return true;
}

void prenderer_vk_chooseDevice(prenderer_t *this, prenderer_init_t spec) {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(this->instance, &deviceCount, NULL);
  VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
  vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices);

  int32_t best  = -1;
  int     score =  0;

  for (size_t i = 0; i < deviceCount; ++i) {
    VkPhysicalDevice device = devices[i];
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    int _score = spec.scoreFunction(deviceProperties, deviceFeatures);
    if (_score > score) { best = i; score = _score; }
  }

  assert(best > 0 && "Failed to pick physical device!");
  if (0 > best) exit(1);

  this->physicalDevice = devices[best];

  // Find queue families
  {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount, NULL);

    VkQueueFamilyProperties *queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(this->physicalDevice, &queueFamilyCount, queueFamilies);

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        this->graphicsFamily = i;
        this->graphicsFamilySet = true;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(this->physicalDevice, i, this->surface, &presentSupport);
      if (presentSupport) {
        this->presentFamily = i;
        this->presentFamilySet = true;
      }

      if (this->graphicsFamilySet && this->presentFamilySet) {
        break;
      }
    }
  }

  assert(this->graphicsFamilySet && this->presentFamilySet && "Couldn't find neither graphics nor present family!");
}

prresult_t prenderer_vk_initialize(prenderer_t *this, prenderer_init_t spec) {
  if (spec.validation) assert(spec.debugCallback && "validationCallback needed but not provided!");
  prresult_t result = {0};

  VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo = {0};
  dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  dbgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  dbgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  dbgCreateInfo.pfnUserCallback = spec.debugCallback;
  dbgCreateInfo.pUserData = NULL;

  {
    VkApplicationInfo appInfo  = {0};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "PurrfectEngine application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "PurrfectEngine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    uint32_t extensionCount = 0;
    const char **extensions = malloc(sizeof(char*) * 12);
    uint32_t layerCount = 0;
    const char **layers = malloc(sizeof(char*) * 12);

    const char **glfwExtensions = getWindowExtensions(&extensionCount);
    for (uint32_t i = 0; i < extensionCount; ++i) extensions[i] = glfwExtensions[i];

    VkInstanceCreateInfo createInfo = {0};
    if (spec.validation) {
      layers[layerCount++] = "VK_LAYER_KHRONOS_validation";
      extensions[extensionCount++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &dbgCreateInfo;
    }

    if (!vkCheckLayerSupport(layers, layerCount)) {
      result.error = PURR_RLAYERS;
      goto defer;
    }

    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount       = layerCount;
    createInfo.ppEnabledLayerNames     = layers;

    VkResult vkResult = vkCreateInstance(&createInfo, NULL, &this->instance);
    if (vkResult != VK_SUCCESS) {
      result.error = PURR_RINSTANCE;
      result.result = vkResult;
      goto defer;
    }

    free(layers);
    free(extensions);
  }

  if (spec.validation) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
      VkResult vkResult = func(this->instance, &dbgCreateInfo, NULL, &this->debugMessenger);
      if (vkResult != VK_SUCCESS) {
        result.error = PURR_RVALIDATION;
        result.result = VK_ERROR_EXTENSION_NOT_PRESENT;
        goto defer;
      }
    } else {
      result.error = PURR_RVALIDATION;
      result.result = VK_ERROR_EXTENSION_NOT_PRESENT;
      goto defer;
    }
  }

  prenderer_vk_chooseDevice(this, spec);

  // Logical device
  {
    VkPhysicalDeviceFeatures deviceFeatures = {0};

    VkDeviceQueueCreateInfo *queueCreateInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * 2);
    float queuePriority = 1.0f;
    uint32_t indices[2] = { this->graphicsFamily, this->presentFamily };
    for (uint32_t i = 0; i < 2; ++i) {
      VkDeviceQueueCreateInfo queueCreateInfo = {0};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = indices[i];
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos[i] = queueCreateInfo;
    }

    VkDeviceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount = 2;
    createInfo.pEnabledFeatures = &deviceFeatures;

    uint32_t layerCount = 0;
    const char **layers = malloc(sizeof(char*) * 12);
    uint32_t extensionCount = 0;
    const char **extensions = malloc(sizeof(char*) * 12);
    if (spec.validation) {
      layers[layerCount++] = "VK_LAYER_KHRONOS_validation";
    }

    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = layerCount;
    createInfo.ppEnabledLayerNames = layers;

    VkResult vkResult = vkCreateDevice(this->physicalDevice, &createInfo, NULL, &this->device);
    if (vkResult != VK_SUCCESS) {
      result.error = PURR_RDEVICE;
      result.result = vkResult;
      goto defer;
    }

    vkGetDeviceQueue(this->device, this->graphicsFamily, 0, &this->graphicsQueue);
    vkGetDeviceQueue(this->device, this->presentFamily,  0, &this->presentQueue);
  }

  // Create surface
  {
    VkResult vkResult = glfwCreateWindowSurface(this->instance, spec.win, NULL, &this->surface);
    if (vkResult != VK_SUCCESS) {
      result.error = PURR_RSURFACE;
      result.result = vkResult;
      goto defer;
    }
  }

defer:
  return result;
}

void prenderer_vk_cleanup(prenderer_t *this) {
  vkDestroyDevice(this->device, NULL);

  if (this->debugMessenger != VK_NULL_HANDLE) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(this->instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
      func(this->instance, this->debugMessenger, NULL);
    }
  }

  vkDestroySurfaceKHR(this->instance, this->surface, NULL);
  vkDestroyInstance(this->instance, NULL);
}

prenderer_t *prenderer_create() {
  prenderer_t *this = malloc(sizeof(prenderer_t));
  memset(this, 0, sizeof(prenderer_t));
  return this;
}

void prenderer_initialize(prenderer_t *this, prenderer_init_t spec) {
  prenderer_vk_initialize(this, spec);
}

void prenderer_delete(prenderer_t *this) {
  prenderer_vk_cleanup(this);
  free(this);
  this = NULL;
}