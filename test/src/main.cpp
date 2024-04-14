#include <PurrfectEngine/renderer.h>

#include <stdio.h>

int score(VkPhysicalDeviceProperties properties, VkPhysicalDeviceFeatures features) {
  return 1000;
}

VkBool32 dbgCallback(VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
                     VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
                     const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
                     void*                                            pUserData) {
  fprintf(stderr, "[VALIDATION]: %s\n", pCallbackData->pMessage);
  return VK_FALSE;
}

int main(void) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window = glfwCreateWindow(800, 600, "test", NULL, NULL);

  prenderer_t *renderer = prenderer_create();
  prenderer_init_t initSpec = {0};
  initSpec.scoreFunction = &score;
  initSpec.validation = true;
  initSpec.debugCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)&dbgCallback;
  initSpec.win = window;
  prenderer_initialize(renderer, initSpec);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  prenderer_delete(renderer);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}