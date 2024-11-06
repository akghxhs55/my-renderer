cmake_minimum_required(VERSION 3.29)
project(my_renderer)

set(CMAKE_CXX_STANDARD 20)

add_executable(my_renderer sources/main.cpp
        sources/my_renderer.cpp
        sources/my_renderer.h)

find_package(VulkanLoader REQUIRED)
target_link_libraries(my_renderer Vulkan::Loader)

find_package(glfw3 REQUIRED)
target_link_libraries(my_renderer glfw)

find_package(glm REQUIRED)
target_link_libraries(my_renderer glm::glm)
