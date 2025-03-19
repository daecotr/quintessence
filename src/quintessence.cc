#include <cstdlib>
#include <iostream>
#include <ostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

int main(int argc, char **argv) {
	try {
		glfwSetErrorCallback([](int, const char *description) { throw std::runtime_error(description); });

		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		GLFWwindow *window = glfwCreateWindow(640, 480, "", nullptr, nullptr);

		vk::ApplicationInfo applicationInfo{"", 0, "", 0, vk::makeApiVersion(0, 1, 3, 296)};
		std::vector<const char *> instanceLayers, instanceExtensions;
		const vk::InstanceCreateInfo instanceCreateInfo{vk::InstanceCreateFlags{},
																				&applicationInfo,
																				static_cast<uint32_t>(instanceLayers.size()),
																				instanceLayers.data(),
																				static_cast<uint32_t>(instanceExtensions.size()),
																				instanceExtensions.data()};
		vk::UniqueInstance instance = vk::createInstanceUnique(instanceCreateInfo);

		glm::mat4 matrix{1};

		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}

		glfwDestroyWindow(window);
		glfwTerminate();
	} catch (const std::exception &exception) {
		std::cerr << "exception.what(): " << exception.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
