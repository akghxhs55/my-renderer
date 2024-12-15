cmake_minimum_required(VERSION 3.29)
project(my_renderer)

set(CMAKE_CXX_STANDARD 20)

add_executable(my_renderer sources/main.cpp
        sources/my_renderer.cpp sources/my_renderer.h
        sources/window.cpp sources/window.h
        sources/environment.cpp sources/environment.h
        sources/vertex.cpp sources/vertex.h
        sources/abstract_buffer.cpp sources/abstract_buffer.h
        sources/device_local_buffer.cpp sources/device_local_buffer.h
        sources/host_visible_buffer.cpp sources/host_visible_buffer.h
        sources/render_pipeline.cpp sources/render_pipeline.h
        sources/abstract_buffer.cpp
)

find_package(VulkanLoader REQUIRED)
target_link_libraries(my_renderer Vulkan::Loader)

find_package(glfw3 REQUIRED)
target_link_libraries(my_renderer glfw)

find_package(glm REQUIRED)
target_link_libraries(my_renderer glm::glm)
