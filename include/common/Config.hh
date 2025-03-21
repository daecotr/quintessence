#pragma once

#include <vulkan/vulkan.hpp>

namespace Q {

#ifdef NDEBUG
constexpr bool DEBUG_MODE = false;
#else  // #ifdef NDEBUG
constexpr bool DEBUG_MODE = true;
#endif // #ifdef NDEBUG #else

constexpr auto VULKAN_API_VERSION = vk::makeApiVersion(0, 1, 3, 296);

} // namespace Q