#include "window/WindowManager.hh"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "common/LogManager.hh"
#include "window/WindowExceptions.hh"

namespace Q {

WindowManager::WindowManager() {
  LogManager::getInstance().write(LogLevel::debug,
                                  "WindowManager initializing...");
  glfwSetErrorCallback([](int, const char *description) {
    LogManager::getInstance().write(
        LogLevel::error, "GLFW error callback: " + std::string(description));
  });

  if (!glfwInit())
    throw WindowException{"Failed to initialize GLFW"};

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  LogManager::getInstance().write(LogLevel::debug, "WindowManager initialized");
}

WindowManager::~WindowManager() {
  LogManager::getInstance().write(LogLevel::debug,
                                  "WindowManager terminating...");
  glfwTerminate();
  LogManager::getInstance().write(LogLevel::debug, "WindowManager terminated");
}

} // namespace Q