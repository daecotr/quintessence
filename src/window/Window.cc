#include "window/Window.hh"

#include "common/LogManager.hh"
#include "window/WindowManager.hh"

namespace Q {

Window::Window(const int width, const int height, const char *title)
    : window{create(width, height, title)} {
  LogManager::getInstance().write(LogLevel::debug, "Window created");
}

Window::UniqueWindow Window::create(const int width, const int height,
                                    const char *title) {
  WindowManager::getInstance();
  GLFWwindow *rawWindow =
      glfwCreateWindow(width, height, title, nullptr, nullptr);
  LogManager::getInstance().write(LogLevel::debug, "Window creating...");
  if (!rawWindow)
    throw std::runtime_error{"Failed to create window"};
  return {rawWindow, &Window::destroy};
}

void Window::destroy(GLFWwindow *window) {
  LogManager::getInstance().write(LogLevel::debug, "Window destroying...");
  glfwDestroyWindow(window);
  LogManager::getInstance().write(LogLevel::debug, "Window destroyed");
}

} // namespace Q
