#include <concepts>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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
    void operator()(const TClass *ptr) const { delete ptr; }
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
bool isAvailable(const std::vector<const char *> &names,
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
      return false;
  }

  return true;
}

bool isInstanceLayersAvailable(const std::vector<const char *> &names) {
  return isAvailable(names, vk::enumerateInstanceLayerProperties());
}

bool isInstanceExtensionsAvailable(const std::vector<const char *> &names) {
  return isAvailable(names, vk::enumerateInstanceExtensionProperties());
}

bool isPhysicalDeviceExtensionsAvailable(
    const vk::PhysicalDevice &physicalDevice,
    const std::vector<const char *> &names) {
  return isAvailable(names,
                     physicalDevice.enumerateDeviceExtensionProperties());
}

void UniqueWindowDestroy(GLFWwindow *window) {
  glfwDestroyWindow(window);
  std::cout << "Window destroyed" << std::endl;
}

using UniqueWindow =
    std::unique_ptr<GLFWwindow, decltype(&UniqueWindowDestroy)>;
UniqueWindow UniqueWindowCreate(const int width, const int height,
                                const char *title = "") {
  GLFWwindow *rawWindow =
      glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!rawWindow)
    throw std::runtime_error("Failed to create `Window`");
  std::cout << "Window created" << std::endl;
  return {rawWindow, UniqueWindowDestroy};
};

std::vector<const char *> instanceLayers = {"VK_LAYER_KHRONOS_validation"};
std::vector<const char *> instanceExtensions = {vk::EXTDebugUtilsExtensionName};

void addGLFWRequiredInstanceExtensions() {
  uint32_t glfwInstanceExtensionCount = 0;
  const char **glfwInstanceExtensions =
      glfwGetRequiredInstanceExtensions(&glfwInstanceExtensionCount);
  instanceExtensions.insert(instanceExtensions.end(), glfwInstanceExtensions,
                            glfwInstanceExtensions +
                                glfwInstanceExtensionCount);
}

vk::UniqueInstance createInstance() {
  vk::ApplicationInfo applicationInfo{"", 0, "", 0,
                                      vk::makeApiVersion(0, 1, 3, 296)};

  addGLFWRequiredInstanceExtensions();
  if (!isInstanceLayersAvailable(instanceLayers))
    throw std::runtime_error{"Not found instance layer"};

  if (!isInstanceExtensionsAvailable(instanceExtensions))
    throw std::runtime_error{"Not found instance extension"};

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
createDebugUtilsMessenger(
    vk::UniqueInstance &instance,
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
            if (pCallbackData->pMessage == nullptr)
              return vk::False;
            std::cout << pCallbackData->pMessage << std::endl;
            return vk::False;
          }};

  return instance->createDebugUtilsMessengerEXTUnique(
      debugUtilsMessengerCreateInfo, nullptr, dispatchLoaderDynamic);
}

const std::vector<const char *> deviceExtensions = {
    vk::KHRSwapchainExtensionName};

std::array<std::optional<uint32_t>, 2>
getPhysicalDeviceQueueFamilyOptionalIndices(
    const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) {
  std::optional<uint32_t> physicalDeviceQueueGraphicsFamily,
      physicalDeviceQueuePresentFamily;
  size_t i = 0;
  auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
  for (const auto &queueFamily : physicalDevice.getQueueFamilyProperties()) {
    if (queueFamily.queueCount > 0 &&
        queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
      physicalDeviceQueueGraphicsFamily = i;
    if (queueFamily.queueCount > 0 &&
        physicalDevice.getSurfaceSupportKHR(i, surface))
      physicalDeviceQueuePresentFamily = i;
    if (physicalDeviceQueueGraphicsFamily.has_value() &&
        physicalDeviceQueuePresentFamily.has_value())
      break;
    ++i;
  }

  return {physicalDeviceQueueGraphicsFamily, physicalDeviceQueuePresentFamily};
}

bool getPhysicalDeviceQueueFamilySupport(
    const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) {
  const auto indices =
      getPhysicalDeviceQueueFamilyOptionalIndices(physicalDevice, surface);
  return indices[0].has_value() && indices[1].has_value();
}

std::set<uint32_t>
getPhysicalDeviceQueueFamilyIndices(const vk::PhysicalDevice &physicalDevice,
                                    const vk::SurfaceKHR &surface) {
  auto physicalDeviceQueueFamilyOptionalIndices =
      getPhysicalDeviceQueueFamilyOptionalIndices(physicalDevice, surface);

  return {physicalDeviceQueueFamilyOptionalIndices[0].value(),
          physicalDeviceQueueFamilyOptionalIndices[1].value()};
}

bool isSuitablePhysicalDevice(const vk::PhysicalDevice &physicalDevice,
                              const vk::SurfaceKHR &surface) {
  return getPhysicalDeviceQueueFamilySupport(physicalDevice, surface) &&
         isPhysicalDeviceExtensionsAvailable(physicalDevice,
                                             deviceExtensions) &&
         !physicalDevice.getSurfaceFormatsKHR(surface).empty() &&
         !physicalDevice.getSurfacePresentModesKHR(surface).empty();
}

vk::PhysicalDevice pickPhysicalDevice(vk::UniqueInstance &instance,
                                      const vk::SurfaceKHR &surface) {
  const auto availablePhysicalDevices = instance->enumeratePhysicalDevices();
  if (availablePhysicalDevices.empty())
    throw std::runtime_error("Failed to enumerate physical devices");

  vk::PhysicalDevice physicalDevice;
  for (const auto &availablePhysicalDevice : availablePhysicalDevices) {
    if (isSuitablePhysicalDevice(availablePhysicalDevice, surface)) {
      physicalDevice = availablePhysicalDevice;
      break;
    }
  }

  if (!physicalDevice)
    throw std::runtime_error("No suitable physical devices");
  return physicalDevice;
}

vk::UniqueDevice createDevice(const vk::PhysicalDevice &physicalDevice,
                              const vk::SurfaceKHR &surface) {
  std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos;
  const std::set<uint32_t> physicalDeviceQueueFamilies =
      getPhysicalDeviceQueueFamilyIndices(physicalDevice, surface);

  float queuePriority = 1.0f;
  deviceQueueCreateInfos.reserve(physicalDeviceQueueFamilies.size());
  for (const auto &physicalDeviceQueueFamily : physicalDeviceQueueFamilies) {
    deviceQueueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{},
                                        physicalDeviceQueueFamily, 1,
                                        &queuePriority);
  }

  vk::PhysicalDeviceFeatures physicalDeviceFeatures{};
  const vk::DeviceCreateInfo deviceCreateInfo{
      vk::DeviceCreateFlags{},
      static_cast<uint32_t>(deviceQueueCreateInfos.size()),
      deviceQueueCreateInfos.data(),
      static_cast<uint32_t>(instanceLayers.size()),
      instanceLayers.data(),
      static_cast<uint32_t>(deviceExtensions.size()),
      deviceExtensions.data(),
      &physicalDeviceFeatures};

  return physicalDevice.createDeviceUnique(deviceCreateInfo);
}

vk::SurfaceFormatKHR
selectSurfaceFormat(std::vector<vk::SurfaceFormatKHR> &surfaceFormats) {
  if (surfaceFormats.size() == 1 &&
      surfaceFormats[0].format == vk::Format::eUndefined) {
    return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
  }

  for (const auto &surfaceFormat : surfaceFormats) {
    if (surfaceFormat.format == vk::Format::eB8G8R8A8Unorm &&
        surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
      return surfaceFormat;
  }

  return surfaceFormats[0];
}

vk::PresentModeKHR
selectSurfacePresentMode(std::vector<vk::PresentModeKHR> &presentModes) {
  auto bestMode = vk::PresentModeKHR::eFifo;

  for (const auto &presentMode : presentModes) {
    if (presentMode == vk::PresentModeKHR::eMailbox)
      return presentMode;
    else if (presentMode == vk::PresentModeKHR::eImmediate)
      bestMode = presentMode;
  }

  return bestMode;
}

vk::Extent2D
selectSurfaceExtent(const vk::SurfaceCapabilitiesKHR &surfaceCapabilities, GLFWwindow* rawWindow) {
  if (surfaceCapabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max())
    return surfaceCapabilities.currentExtent;
  else {
    int width, height;
    glfwGetFramebufferSize(rawWindow, &width, &height);
    vk::Extent2D actualExtent = {static_cast<uint32_t>(width),
                                 static_cast<uint32_t>(height)};

    actualExtent.width = std::max(
        surfaceCapabilities.minImageExtent.width,
        std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height =
        std::max(surfaceCapabilities.minImageExtent.height,
                 std::min(surfaceCapabilities.maxImageExtent.height,
                          actualExtent.height));

    return actualExtent;
  }
}

vk::SurfaceFormatKHR getSurfaceFormat(const vk::PhysicalDevice &physicalDevice,
                                      const vk::SurfaceKHR &surface) {
  auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
  return selectSurfaceFormat(surfaceFormats);
}

vk::UniqueSwapchainKHR createSwapchain(vk::UniqueDevice &device,
                                       const vk::PhysicalDevice physicalDevice,
                                       const vk::SurfaceKHR &surface, GLFWwindow* rawWindow) {
  const auto surfaceFormat = getSurfaceFormat(physicalDevice, surface);
  auto surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);
  const auto surfacePresentMode = selectSurfacePresentMode(surfacePresentModes);
  const auto surfaceCapabilities =
      physicalDevice.getSurfaceCapabilitiesKHR(surface);
  const auto surfaceExtent = selectSurfaceExtent(surfaceCapabilities, rawWindow);

  auto minImageCount = surfaceCapabilities.minImageCount + 1;
  if (surfaceCapabilities.maxImageCount > 0 &&
      minImageCount > surfaceCapabilities.maxImageCount)
    minImageCount = surfaceCapabilities.maxImageCount;

  auto imageSharingMode = vk::SharingMode::eExclusive;
  uint32_t queueFamilyIndexCount = 0;
  uint32_t queueFamilyIndices[2];

  if (const auto physicalDeviceQueueFamilyOptionalIndices =
          getPhysicalDeviceQueueFamilyOptionalIndices(physicalDevice, surface);
      physicalDeviceQueueFamilyOptionalIndices[0] !=
      physicalDeviceQueueFamilyOptionalIndices[1]) {
    imageSharingMode = vk::SharingMode::eConcurrent;
    queueFamilyIndexCount = 2;
    queueFamilyIndices[0] = physicalDeviceQueueFamilyOptionalIndices[0].value();
    queueFamilyIndices[1] = physicalDeviceQueueFamilyOptionalIndices[1].value();
  }

  const vk::SwapchainCreateInfoKHR swapchainCreateInfo{
      vk::SwapchainCreateFlagsKHR{},
      surface,
      minImageCount,
      surfaceFormat.format,
      surfaceFormat.colorSpace,
      surfaceExtent,
      1,
      vk::ImageUsageFlagBits::eColorAttachment,
      imageSharingMode,
      queueFamilyIndexCount,
      queueFamilyIndices,
      surfaceCapabilities.currentTransform,
      vk::CompositeAlphaFlagBitsKHR::eOpaque,
      surfacePresentMode,
      vk::True,
      vk::SwapchainKHR{nullptr}};

  return device->createSwapchainKHRUnique(swapchainCreateInfo);
}

std::vector<vk::UniqueImageView>
getSwapchainImageViews(const vk::UniqueDevice &device,
                       const std::vector<vk::Image> &swapchainImages,
                       const vk::Format &swapchainSurfaceFormat) {
  std::vector<vk::UniqueImageView> swapchainImageViews;
  swapchainImageViews.reserve(swapchainImages.size());

  for (const auto &swapchainImage : swapchainImages) {
    vk::ImageViewCreateInfo imageViewCreateInfo{
        vk::ImageViewCreateFlags{},
        swapchainImage,
        vk::ImageViewType::e2D,
        swapchainSurfaceFormat,
        vk::ComponentMapping{},
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
    swapchainImageViews.emplace_back(
        device->createImageViewUnique(imageViewCreateInfo));
  }

  return std::move(swapchainImageViews);
}

vk::UniqueRenderPass createRenderPass(const vk::UniqueDevice &device,
                                      const vk::Format swapchainImageFormat) {
  vk::AttachmentDescription attachmentDescriptions{
      vk::AttachmentDescriptionFlags{}, swapchainImageFormat,
      vk::SampleCountFlagBits::e1,      vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore,    vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
      vk::ImageLayout::ePresentSrcKHR};

  vk::AttachmentReference attachmentReference{
      0, vk::ImageLayout::eColorAttachmentOptimal};

  vk::SubpassDescription subpassDescription{vk::SubpassDescriptionFlags{},
                                            vk::PipelineBindPoint::eGraphics,
                                            0,
                                            nullptr,
                                            1,
                                            &attachmentReference};

  vk::SubpassDependency subpassDependency{
      vk::SubpassExternal,
      0,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::AccessFlags{},
      vk::AccessFlagBits::eColorAttachmentRead |
          vk::AccessFlagBits::eColorAttachmentWrite};

  const vk::RenderPassCreateInfo renderPassCreateInfo{
      vk::RenderPassCreateFlags{}, 1, &attachmentDescriptions, 1,
      &subpassDescription,         1, &subpassDependency};

  return device->createRenderPassUnique(renderPassCreateInfo);
}

std::vector<char> readFile(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::ate | std::ios::binary);
  if (!file.is_open())
    throw std::runtime_error("Could not open file " + filePath);

  const std::streamsize fileSize = static_cast<std::streamsize>(file.tellg());
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();
  return buffer;
}

vk::UniqueShaderModule createShaderModule(const vk::UniqueDevice &device,
                                          const std::vector<char> &code) {
  return device->createShaderModuleUnique(
      {vk::ShaderModuleCreateFlags(), code.size(),
       reinterpret_cast<const uint32_t *>(code.data())});
}

vk::UniquePipelineLayout createPipelineLayout(const vk::UniqueDevice &device) {
  constexpr vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
      vk::PipelineLayoutCreateFlags{}, 0, nullptr, 0, nullptr};
  return device->createPipelineLayoutUnique(pipelineLayoutCreateInfo);
}

vk::UniquePipeline createGraphicsPipeline(
    const vk::UniqueDevice &device, vk::UniqueRenderPass &renderPass,
    vk::UniquePipelineLayout &pipelineLayout, vk::Extent2D &swapchainExtent) {
  auto vertexShaderCode = readFile("assets/shaders/vertex.spv");
  auto fragmentShaderCode = readFile("assets/shaders/fragment.spv");
  auto vertexShaderModule = createShaderModule(device, vertexShaderCode);
  auto fragmentShaderModule = createShaderModule(device, fragmentShaderCode);
  vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[] = {
      {vk::PipelineShaderStageCreateFlags{}, vk::ShaderStageFlagBits::eVertex,
       *vertexShaderModule, "main"},
      {vk::PipelineShaderStageCreateFlags{}, vk::ShaderStageFlagBits::eFragment,
       *fragmentShaderModule, "main"}};
  vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{
      vk::PipelineVertexInputStateCreateFlags{}, 0, nullptr, 0, nullptr};
  vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{
      vk::PipelineInputAssemblyStateCreateFlags{},
      vk::PrimitiveTopology::eTriangleList, vk::False};
  vk::Viewport viewport{0,
                        0,
                        static_cast<float>(swapchainExtent.width),
                        static_cast<float>(swapchainExtent.height),
                        0,
                        1};
  vk::Rect2D scissor{vk::Offset2D{0, 0}, swapchainExtent};
  vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{
      vk::PipelineViewportStateCreateFlags{}, 1, &viewport, 1, &scissor};
  vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{
      vk::PipelineRasterizationStateCreateFlags{},
      vk::False,
      vk::False,
      vk::PolygonMode::eFill,
      vk::CullModeFlagBits::eBack,
      vk::FrontFace::eClockwise,
      vk::False,
      {},
      {},
      {},
      1};
  vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{
      vk::PipelineMultisampleStateCreateFlags{}, vk::SampleCountFlagBits::e1,
      vk::False};
  vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{
      vk::False,
      vk::BlendFactor::eZero,
      vk::BlendFactor::eZero,
      vk::BlendOp::eAdd,
      vk::BlendFactor::eZero,
      vk::BlendFactor::eZero,
      vk::BlendOp::eAdd,
      vk::ColorComponentFlags{
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA}};
  vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{
      vk::PipelineColorBlendStateCreateFlags{},
      vk::False,
      vk::LogicOp::eCopy,
      1,
      &colorBlendAttachmentState,
      std::array<float, 4>{0, 0, 0, 0}};
  vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
      vk::PipelineCreateFlags{},
      2,
      pipelineShaderStageCreateInfos,
      &pipelineVertexInputStateCreateInfo,
      &pipelineInputAssemblyStateCreateInfo,
      nullptr,
      &pipelineViewportStateCreateInfo,
      &pipelineRasterizationStateCreateInfo,
      &pipelineMultisampleStateCreateInfo,
      nullptr,
      &pipelineColorBlendStateCreateInfo,
      nullptr,
      *pipelineLayout,
      *renderPass,
      0,
      nullptr};
  return device
      ->createGraphicsPipelineUnique(nullptr, graphicsPipelineCreateInfo)
      .value;
}

std::vector<vk::UniqueFramebuffer> createFramebuffers(
    vk::UniqueDevice &device, std::vector<vk::UniqueImageView> &imageViews,
    vk::UniqueRenderPass &renderPass, const vk::Extent2D &swapchainExtent) {
  std::vector<vk::UniqueFramebuffer> swapchainFramebuffers;
  swapchainFramebuffers.reserve(imageViews.size());
  for (auto &imageView : imageViews) {
    vk::ImageView attachments[] = {*imageView};
    vk::FramebufferCreateInfo framebufferCreateInfo{
        vk::FramebufferCreateFlags{},
        *renderPass,
        1,
        attachments,
        swapchainExtent.width,
        swapchainExtent.height,
        1};
    swapchainFramebuffers.emplace_back(
        device->createFramebufferUnique(framebufferCreateInfo));
  }

  return std::move(swapchainFramebuffers);
}

vk::UniqueCommandPool
createCommandPool(vk::UniqueDevice &device,
                  const vk::PhysicalDevice &physicalDevice,
                  const vk::SurfaceKHR &surface) {
  const auto queueFamilyOptionalIndices =
      getPhysicalDeviceQueueFamilyOptionalIndices(physicalDevice, surface);
  const vk::CommandPoolCreateInfo commandPoolCreateInfo{
      vk::CommandPoolCreateFlags{}, queueFamilyOptionalIndices[0].value()};
  return device->createCommandPoolUnique(commandPoolCreateInfo);
}

std::vector<vk::UniqueCommandBuffer> createCommandBuffers(
    vk::UniqueDevice &device,
    const std::vector<vk::UniqueFramebuffer> &framebuffers,
    vk::UniqueCommandPool &commandPool, vk::UniqueRenderPass &renderPass,
    const vk::Extent2D &swapchainExtent, vk::UniquePipeline &graphicsPipeline) {
  std::vector<vk::UniqueCommandBuffer> commandBuffers;
  commandBuffers.resize(framebuffers.size());
  const vk::CommandBufferAllocateInfo commandBufferAllocateInfo{
      *commandPool, vk::CommandBufferLevel::ePrimary,
      static_cast<uint32_t>(framebuffers.size())};
  commandBuffers =
      device->allocateCommandBuffersUnique(commandBufferAllocateInfo);

  for (uint32_t i = 0; i < commandBuffers.size(); ++i) {
    vk::CommandBufferBeginInfo commandBufferBeginInfo{
        vk::CommandBufferUsageFlagBits::eSimultaneousUse};

    commandBuffers[i]->begin(commandBufferBeginInfo);

    vk::ClearValue clearColor{std::array<float, 4>{0, 0, 0, 0}};
    vk::RenderPassBeginInfo renderPassBeginInfo{
        *renderPass, *framebuffers[i],
        vk::Rect2D{vk::Offset2D{0, 0}, swapchainExtent}, 1, &clearColor};

    commandBuffers[i]->beginRenderPass(renderPassBeginInfo,
                                       vk::SubpassContents::eInline);
    commandBuffers[i]->bindPipeline(vk::PipelineBindPoint::eGraphics,
                                    *graphicsPipeline);
    commandBuffers[i]->draw(3, 1, 0, 0);
    commandBuffers[i]->endRenderPass();
    commandBuffers[i]->end();
  }

  return std::move(commandBuffers);
}

void recreateSwapchain(vk::UniqueDevice &device, GLFWwindow *window,
                       vk::UniqueSwapchainKHR &swapchain,
                       std::vector<vk::UniqueImageView> &imageViews,
                       std::vector<vk::UniqueFramebuffer> &framebuffers,
                       const vk::PhysicalDevice &physicalDevice,
                       const vk::SurfaceKHR &surface,
                       std::vector<vk::Image> &swapchainImages,
                       const vk::Format &swapchainSurfaceFormat,
                       vk::UniqueRenderPass &renderPass,
                       const vk::Extent2D &swapchainExtent,
                       std::vector<vk::UniqueCommandBuffer> &commandBuffers,
                       vk::UniqueCommandPool &commandPool,
                       vk::Extent2D swapchainSurfaceExtent,
                       vk::UniquePipeline &graphicsPipeline) {
  int width = 0, height = 0;
  while (width == 0 || height == 0) {
    glfwGetWindowSize(window, &width, &height);
    glfwWaitEvents();
  }

  device->waitIdle();
  swapchainSurfaceExtent = selectSurfaceExtent(
        physicalDevice.getSurfaceCapabilitiesKHR(surface), window);
  swapchain.reset();
  swapchain = createSwapchain(device, physicalDevice, surface, window);
  swapchainImages.clear();
  swapchainImages = device->getSwapchainImagesKHR(*swapchain);
  imageViews.clear();
  imageViews =
      getSwapchainImageViews(device, swapchainImages, swapchainSurfaceFormat);
  framebuffers.clear();
  framebuffers =
      createFramebuffers(device, imageViews, renderPass, swapchainExtent);
  commandBuffers.clear();
  commandBuffers =
      Q::createCommandBuffers(device, framebuffers, commandPool, renderPass,
                              swapchainSurfaceExtent, graphicsPipeline);
}

} // namespace Q

int main(int argc, char **argv) {
  try {
    Q::WindowManager::getInstance();
    auto window{Q::UniqueWindowCreate(64, 64)};

    auto instance = Q::createInstance();
    const vk::DispatchLoaderDynamic dispatchLoaderDynamic{
        *instance, vkGetInstanceProcAddr};
    auto debugUtilsMessenger =
        Q::createDebugUtilsMessenger(instance, dispatchLoaderDynamic);

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

    const auto surface{UniqueSurfaceCreate()};

    const auto physicalDevice = Q::pickPhysicalDevice(instance, *surface.get());
    auto device = Q::createDevice(physicalDevice, *surface.get());
    auto swapchain = Q::createSwapchain(device, physicalDevice, *surface.get(), window.get());
    auto swapchainImages = device->getSwapchainImagesKHR(*swapchain);
    const auto swapchainSurfaceFormat =
        Q::getSurfaceFormat(physicalDevice, *surface.get()).format;
    auto swapchainSurfaceExtent = Q::selectSurfaceExtent(
        physicalDevice.getSurfaceCapabilitiesKHR(*surface.get()), window.get());
    auto swapchainImageViews = Q::getSwapchainImageViews(
        device, swapchainImages, swapchainSurfaceFormat);
    auto renderPass = Q::createRenderPass(device, swapchainSurfaceFormat);
    auto pipelineLayout = Q::createPipelineLayout(device);
    auto graphicsPipeline = Q::createGraphicsPipeline(
        device, renderPass, pipelineLayout, swapchainSurfaceExtent);
    auto framebuffers = Q::createFramebuffers(
        device, swapchainImageViews, renderPass, swapchainSurfaceExtent);
    auto commandPool =
        Q::createCommandPool(device, physicalDevice, *surface.get());
    auto commandBuffers =
        Q::createCommandBuffers(device, framebuffers, commandPool, renderPass,
                                swapchainSurfaceExtent, graphicsPipeline);

    auto physicalDeviceQueueFamilyOptionalIndices =
        Q::getPhysicalDeviceQueueFamilyOptionalIndices(physicalDevice,
                                                       *surface.get());
    auto graphicsQueue = device->getQueue(
        physicalDeviceQueueFamilyOptionalIndices[0].value(), 0);
    auto presentQueue = device->getQueue(
        physicalDeviceQueueFamilyOptionalIndices[1].value(), 0);

    constexpr auto maxFramesInFlight = 2;
    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores{};
    imageAvailableSemaphores.reserve(maxFramesInFlight);
    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores{};
    renderFinishedSemaphores.reserve(maxFramesInFlight);
    std::vector<vk::UniqueFence> inFlightFences{};
    inFlightFences.reserve(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; ++i) {
      imageAvailableSemaphores.emplace_back(device->createSemaphoreUnique({}));
      renderFinishedSemaphores.emplace_back(device->createSemaphoreUnique({}));
      inFlightFences.emplace_back(
          device->createFenceUnique({vk::FenceCreateFlagBits::eSignaled}));
    }

    bool framebufferResized = false;
    glfwSetWindowUserPointer(window.get(), &framebufferResized);
    glfwSetFramebufferSizeCallback(
        window.get(), [](GLFWwindow *rawWindow, int, int) {
          *static_cast<bool *>(glfwGetWindowUserPointer(rawWindow)) = true;
        });

    uint32_t currentFrame = 0;
    vk::Result result;
    while (!glfwWindowShouldClose(window.get())) {
      glfwPollEvents();
      result =
          device->waitForFences(1, &*inFlightFences[currentFrame], vk::True,
                                std::numeric_limits<uint64_t>::max());
      result = device->resetFences(1, &*inFlightFences[currentFrame]);
      uint32_t imageIndex =
          device
              ->acquireNextImageKHR(
                  *swapchain, std::numeric_limits<uint64_t>::max(),
                  *imageAvailableSemaphores[currentFrame], nullptr)
              .value;

      vk::Semaphore waitSemaphores[] = {
          *imageAvailableSemaphores[currentFrame]};
      vk::PipelineStageFlags waitStages[] = {
          vk::PipelineStageFlagBits::eColorAttachmentOutput};
      vk::Semaphore signalSemaphores[] = {
          *renderFinishedSemaphores[currentFrame]};
      vk::SubmitInfo submitInfo{
          1, waitSemaphores,  waitStages, 1, &*commandBuffers[imageIndex],
          1, signalSemaphores};
      graphicsQueue.submit(submitInfo, *inFlightFences[currentFrame]);

      vk::SwapchainKHR swapchains[] = {*swapchain};
      vk::PresentInfoKHR presentInfo{1,          signalSemaphores, 1,
                                     swapchains, &imageIndex,      &result};
      result = presentQueue.presentKHR(presentInfo);
      if (result == vk::Result::eErrorOutOfDateKHR ||
          result == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        Q::recreateSwapchain(
            device, window.get(), swapchain, swapchainImageViews, framebuffers,
            physicalDevice, *surface.get(), swapchainImages,
            swapchainSurfaceFormat, renderPass, swapchainSurfaceExtent,
            commandBuffers, commandPool, swapchainSurfaceExtent,
            graphicsPipeline);
      }

      currentFrame = (currentFrame + 1) % maxFramesInFlight;
    }

    device->waitIdle();
  } catch (const std::exception &exception) {
    std::cerr << exception.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl;
  }

  return EXIT_SUCCESS;
}
