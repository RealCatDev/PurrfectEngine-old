#ifndef   PURRENGINE_RENDERER_H_
#define   PURRENGINE_RENDERER_H_

#include <stdbool.h>

#ifdef _WIN32
  #define VK_USE_PLATFORM_WIN32_KHR
  #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int(*prscore_func)(VkPhysicalDeviceProperties, VkPhysicalDeviceFeatures);

typedef enum {
  PURR_RSUCCESS    = 0,
  PURR_RINSTANCE   = 1,
  PURR_RLAYERS     = 2,
  PURR_RVALIDATION = 3,
  PURR_RDEVICE     = 4,
  PURR_RSURFACE    = 5,
} prerror_t;

typedef struct {
  prerror_t error;
  VkResult result;
} prresult_t;

typedef struct {
  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice;
  bool graphicsFamilySet;
  bool presentFamilySet;
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  VkQueue  graphicsQueue;
  VkQueue  presentQueue;
  VkDevice device;
  VkSurfaceKHR surface;
} prenderer_t;

typedef struct {
  GLFWwindow                          *win;
  bool                                 validation;
  PFN_vkDebugUtilsMessengerCallbackEXT debugCallback;
  prscore_func                         scoreFunction;
} prenderer_init_t;

prenderer_t *prenderer_create();
void         prenderer_initialize(prenderer_t *, prenderer_init_t spec);
void         prenderer_delete(prenderer_t *);

#ifdef __cplusplus
}
#endif

#endif // PURRENGINE_RENDERER_H_