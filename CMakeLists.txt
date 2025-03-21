cmake_minimum_required(VERSION 3.30)
project(quintessence)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompileShaders)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(glfw3_LIBRARIES glfw)

add_executable(${PROJECT_NAME}
  src/quintessence.cc
  include/common/Singleton.hh
  include/common/ILogger.hh
  include/common/Exception.hh
  src/common/Exception.cc
  include/common/LogManager.hh
  src/common/LogManager.cc
  src/common/TerminalLogger.cc
  include/common/TerminalLogger.hh
  include/common/Config.hh
  src/window/Window.cc
  include/window/Window.hh
  src/window/WindowManager.cc
  include/window/WindowManager.hh
  include/window/WindowExceptions.hh
)

target_include_directories(${PROJECT_NAME} PRIVATE
  include
  ${glfw3_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIRS}
  ${glm_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${glfw3_LIBRARIES}
  ${Vulkan_LIBRARIES}
  ${glm_LIBRARIES}
)

compile_shaders(
  SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
  OUTPUT_DIR "${CMAKE_BINARY_DIR}/assets/shaders"
  TARGET_NAME "shaders_target"
  SHADER_TYPES "vert" "frag"
)

add_dependencies(${PROJECT_NAME} shaders_target)