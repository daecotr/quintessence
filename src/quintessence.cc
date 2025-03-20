#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>
#include <vulkan/vulkan.hpp>

int main(int argc, char **argv) {
  try {
    glfwSetErrorCallback([](int, const char *description) {
      throw std::runtime_error(description);
    });

    if (!glfwInit())
      throw std::runtime_error("Failed to initialize `WindowManager`");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(512, 512, "", nullptr, nullptr);
    if (!window)
      throw std::runtime_error("Failed to create `Window`");

    vk::ApplicationInfo applicationInfo{"", 0, "", 0,
                                        vk::makeApiVersion(0, 1, 3, 296)};

    std::vector<const char *> instanceLayers = {"VK_LAYER_KHRONOS_validation"},
                              instanceExtensions = {
                                  vk::EXTDebugUtilsExtensionName};

    const auto availableInstanceLayers = vk::enumerateInstanceLayerProperties();
    for (const auto &instanceLayer : instanceLayers) {
      bool instanceLayerFound = false;
      for (const auto &availableInstanceLayer : availableInstanceLayers) {
        if (std::strcmp(availableInstanceLayer.layerName, instanceLayer) == 0) {
          instanceLayerFound = true;
          break;
        }
      }
      if (!instanceLayerFound)
        throw std::runtime_error("Failed to find required instance layer " +
                                 std::string{instanceLayer});
    }

    uint32_t glfwInstanceExtensionCount = 0;
    const char **glfwInstanceExtensions =
        glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionCount);
    instanceExtensions.insert(instanceExtensions.end(), glfwInstanceExtensions,
                              glfwInstanceExtensions +
                                  glfwInstanceExtensionCount);

    const auto availableInstanceExtensions =
        vk::enumerateInstanceExtensionProperties();
    for (const auto &instanceExtension : instanceExtensions) {
      bool instanceExtensionFound = false;
      for (const auto &availableInstanceExtension :
           availableInstanceExtensions) {
        if (std::strcmp(availableInstanceExtension.extensionName,
                        instanceExtension) == 0) {
          instanceExtensionFound = true;
          break;
        }
      }
      if (!instanceExtensionFound)
        throw std::runtime_error("Failed to find required instance extension " +
                                 std::string{instanceExtension});
    }

    const vk::InstanceCreateInfo instanceCreateInfo{
        vk::InstanceCreateFlags{},
        &applicationInfo,
        static_cast<uint32_t>(instanceLayers.size()),
        instanceLayers.data(),
        static_cast<uint32_t>(instanceExtensions.size()),
        instanceExtensions.data()};

    vk::UniqueInstance instance = vk::createInstanceUnique(instanceCreateInfo);

    constexpr vk::DebugUtilsMessengerCreateInfoEXT
        debugUtilsMessengerCreateInfo{
            {},
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
            [](VkDebugUtilsMessageSeverityFlagBitsEXT,
               VkDebugUtilsMessageTypeFlagsEXT,
               const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
               void *) VKAPI_ATTR -> vk::Bool32 {
              std::cout << pCallbackData->pMessage << std::endl;
              return vk::False;
            }};

    const vk::DispatchLoaderDynamic dispatchLoaderDynamic{
        *instance, vkGetInstanceProcAddr};
    auto debugUtilsMessenger = instance->createDebugUtilsMessengerEXTUnique(
        debugUtilsMessengerCreateInfo, nullptr, dispatchLoaderDynamic);

    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &rawSurface) !=
        VK_SUCCESS)
      throw std::runtime_error("Failed to create window surface");
    const vk::SurfaceKHR surface = rawSurface;

    const auto availableDevices = instance->enumeratePhysicalDevices();
    if (availableDevices.empty())
      throw std::runtime_error("Failed to enumerate physical devices");

    const std::vector<const char *> deviceExtensions = {
        vk::KHRSwapchainExtensionName};

    glfwPollEvents();

    instance->destroySurfaceKHR(surface);

    glfwDestroyWindow(window);
    glfwTerminate();
  } catch (const std::exception &exception) {
    std::cerr << exception.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
