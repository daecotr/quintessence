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
		glfwSetErrorCallback([](int, const char *description) { throw std::runtime_error(description); });

		if (!glfwInit())
			throw std::runtime_error("Failed to initialize `WindowManager`");
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		GLFWwindow *window = glfwCreateWindow(512, 512, "", nullptr, nullptr);
		if (!window)
			throw std::runtime_error("Failed to create `Window`");

		glfwPollEvents();

		glfwDestroyWindow(window);
		glfwTerminate();
	} catch (const std::exception &exception) {
		std::cerr << exception.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
