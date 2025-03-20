cmake_minimum_required(VERSION 3.30)
project(quintessence)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

find_package(Vulkan REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
find_package(glm REQUIRED)

set(ASSETS_DIR "assets")
set(SHADERS_DIR "${ASSETS_DIR}/shaders")

find_program(GLSLANG_VALIDATOR glslangValidator REQUIRED)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${SHADERS_DIR})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${SHADERS_DIR}/vertex.spv
  COMMAND ${GLSLANG_VALIDATOR} -V
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/main.vert
  -o ${CMAKE_CURRENT_BINARY_DIR}/${SHADERS_DIR}/vertex.spv
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/main.vert
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${SHADERS_DIR}/fragment.spv
  COMMAND ${GLSLANG_VALIDATOR} -V
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders/main.frag
  -o ${CMAKE_CURRENT_BINARY_DIR}/${SHADERS_DIR}/fragment.spv
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/main.frag
)

add_custom_target(
  compile_shaders ALL
  DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/${SHADERS_DIR}/vertex.spv
  ${CMAKE_CURRENT_BINARY_DIR}/${SHADERS_DIR}/fragment.spv
)

add_executable(${PROJECT_NAME}
  src/quintessence.cc
)

target_include_directories(${PROJECT_NAME} PRIVATE
  include
  ${GLFW_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIRS}
  ${glm_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${GLFW_LIBRARIES}
  ${Vulkan_LIBRARIES}
  ${glm_LIBRARIES}
)

add_dependencies(${PROJECT_NAME} compile_shaders)