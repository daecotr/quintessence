#include <concepts>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace Q {

template <class TClass> class Singleton {
public:
  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;

  static TClass &getInstance();

protected:
  Singleton() = default;
  ~Singleton() = default;

private:
  struct Deleter {
    void operator()(TClass *ptr) const { delete ptr; }
  };

  using UniqueTClass = std::unique_ptr<TClass, Deleter>;

  static UniqueTClass &getStorage();
};

template <class TClass> TClass &Singleton<TClass>::getInstance() {
  UniqueTClass &storage = getStorage();
  if (!storage)
    storage.reset(new TClass());
  return *storage;
}

template <class TClass>
typename Singleton<TClass>::UniqueTClass &Singleton<TClass>::getStorage() {
  static UniqueTClass storage;
  return storage;
}

class WindowManager : public Singleton<WindowManager> {
  friend class Singleton<WindowManager>;

protected:
  WindowManager() {
    glfwSetErrorCallback([](int, const char *description) {
      throw std::runtime_error(description);
    });

    if (!glfwInit())
      throw std::runtime_error("Failed to initialize `WindowManager`");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    std::cout << "WindowManager initialized" << std::endl;
  }

  ~WindowManager() {
    glfwTerminate();
    std::cout << "WindowManager destroyed" << std::endl;
  }
};

std::string getNameOfEnumerated(const vk::LayerProperties &layer) {
  return layer.layerName;
}

std::string getNameOfEnumerated(const vk::ExtensionProperties &extension) {
  return extension.extensionName;
}

template <typename TAvailableProperties>
concept AvailableProperties =
    std::ranges::input_range<TAvailableProperties> &&
    requires(std::ranges::range_reference_t<TAvailableProperties>
                 availableProperty) {
      {
        getNameOfEnumerated(availableProperty)
      } -> std::convertible_to<std::string>;
    };

template <typename TAvailableProperties>
void checkAvailable(const std::vector<const char *> &names,
                    const TAvailableProperties &availableProperties) {
  for (const auto &name : names) {
    bool found = false;
    for (const auto &availableProperty : availableProperties) {
      if (std::strcmp(getNameOfEnumerated(availableProperty).c_str(), name) ==
          0) {
        found = true;
        break;
      }
    }
    if (!found)
      throw std::runtime_error(
          "Failed to find required instance layer/extension " +
          std::string{name});
  }
}

void checkLayersAvailable(const std::vector<const char *> &names) {
  checkAvailable(names, vk::enumerateInstanceLayerProperties());
}

void checkExtensionsAvailable(const std::vector<const char *> &names) {
  checkAvailable(names, vk::enumerateInstanceExtensionProperties());
}

void UniqueWindowDestroy(GLFWwindow *window) {
  glfwDestroyWindow(window);
  std::cout << "Window destroyed" << std::endl;
}

using UniqueWindow =
    std::unique_ptr<GLFWwindow, decltype(&UniqueWindowDestroy)>;
UniqueWindow UniqueWindowCreate(int width, int height, const char *title = "") {
  GLFWwindow *rawWindow =
      glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!rawWindow)
    throw std::runtime_error("Failed to create `Window`");
  std::cout << "Window created" << std::endl;
  return {rawWindow, UniqueWindowDestroy};
};

std::vector<const char *> getInstanceLayers() {
  std::vector<const char *> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
  checkLayersAvailable(instanceLayers);
  return instanceLayers;
}

std::vector<const char *> getInstanceExtensions() {
  std::vector<const char *> instanceExtensions = {
      vk::EXTDebugUtilsExtensionName};
  checkExtensionsAvailable(instanceExtensions);

  uint32_t glfwInstanceExtensionCount = 0;
  const char **glfwInstanceExtensions =
      glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionCount);
  instanceExtensions.insert(instanceExtensions.end(), glfwInstanceExtensions,
                            glfwInstanceExtensions +
                                glfwInstanceExtensionCount);

  return instanceExtensions;
}

vk::UniqueInstance createInstance() {
  vk::ApplicationInfo applicationInfo{"", 0, "", 0,
                                      vk::makeApiVersion(0, 1, 3, 296)};

  std::vector<const char *> instanceLayers = getInstanceLayers(),
                            instanceExtensions = getInstanceExtensions();

  const vk::InstanceCreateInfo instanceCreateInfo{
      vk::InstanceCreateFlags{},
      &applicationInfo,
      static_cast<uint32_t>(instanceLayers.size()),
      instanceLayers.data(),
      static_cast<uint32_t>(instanceExtensions.size()),
      instanceExtensions.data()};

  return vk::createInstanceUnique(instanceCreateInfo);
}

vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>
createDebugUtilsMessenger(vk::UniqueInstance &instance,
    const vk::DispatchLoaderDynamic &dispatchLoaderDynamic) {
  constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
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
         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
          VKAPI_ATTR -> vk::Bool32 {
            std::cout << pCallbackData->pMessage << std::endl;
            return vk::False;
          }};

  return instance->createDebugUtilsMessengerEXTUnique(
      debugUtilsMessengerCreateInfo, nullptr, dispatchLoaderDynamic);
}
} // namespace Q

int main(int argc, char **argv) {
  try {
    Q::WindowManager::getInstance();
    Q::UniqueWindow window{Q::UniqueWindowCreate(512, 512)};

    auto instance = Q::createInstance();
    const vk::DispatchLoaderDynamic dispatchLoaderDynamic{*instance,
                                                      vkGetInstanceProcAddr};
    auto debugUtilsMessenger = Q::createDebugUtilsMessenger(instance, dispatchLoaderDynamic);

    auto UniqueSurfaceDestroy =
        [&instance](const vk::SurfaceKHR *surfaceBeingDestroyed) {
          instance->destroySurfaceKHR(*surfaceBeingDestroyed);
          std::cout << "Surface destroyed" << std::endl;
        };

    using UniqueSurface =
        std::unique_ptr<vk::SurfaceKHR, decltype(UniqueSurfaceDestroy)>;

    auto UniqueSurfaceCreate = [&UniqueSurfaceDestroy, &instance,
                                &window]() -> UniqueSurface {
      VkSurfaceKHR rawSurface;
      if (glfwCreateWindowSurface(*instance, window.get(), nullptr,
                                  &rawSurface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
      // ReSharper disable once CppDFAMemoryLeak (UniqueSurfaceDestroy release
      // memory)
      const auto surface = new vk::SurfaceKHR{rawSurface};
      std::cout << "Surface created" << std::endl;
      return UniqueSurface{surface, UniqueSurfaceDestroy};
    };

    UniqueSurface surface{UniqueSurfaceCreate()};

    const auto availableDevices = instance->enumeratePhysicalDevices();
    if (availableDevices.empty())
      throw std::runtime_error("Failed to enumerate physical devices");

    const std::vector<const char *> deviceExtensions = {
        vk::KHRSwapchainExtensionName};

    glfwPollEvents();

  } catch (const std::exception &exception) {
    std::cerr << exception.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
