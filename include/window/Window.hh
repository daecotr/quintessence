#pragma once

#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Q {

class Window {
public:
  Window(int width, int height, const char *title = "");
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

private:
  static void destroy(GLFWwindow *window);
  using UniqueWindow =
      std::unique_ptr<GLFWwindow, decltype(&Window::destroy)>;
  UniqueWindow window;
  static UniqueWindow create(int width, int height, const char *title);
};

} // namespace Q