cmake_minimum_required(VERSION 3.10)
project(quintessence LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
find_package(glm REQUIRED)

set(SOURCES src)

add_executable(${PROJECT_NAME}
	${SOURCES}/${PROJECT_NAME}.cc
)

target_include_directories(${PROJECT_NAME} PRIVATE
	include
  ${GLFW_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIRS}
	${glm_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${GLFW_LIBRARIES}
  Vulkan::Vulkan
	glm::glm
)
