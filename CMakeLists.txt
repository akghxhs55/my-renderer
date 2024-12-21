cmake_minimum_required(VERSION 3.29)
project(my_renderer)

set(CMAKE_CXX_STANDARD 20)

add_executable(my_renderer sources/main.cpp
        sources/my_renderer.cpp sources/my_renderer.h
        sources/utils/window.cpp sources/utils/window.h
        sources/utils/environment.cpp sources/utils/environment.h
        sources/utils/render_pipeline.cpp sources/utils/render_pipeline.h
        sources/utils/i_buffer.h
        sources/utils/abstract_buffer.cpp sources/utils/abstract_buffer.h
        sources/utils/device_local_buffer.cpp sources/utils/device_local_buffer.h
        sources/utils/host_visible_buffer.cpp sources/utils/host_visible_buffer.h
        sources/vertex.cpp sources/vertex.h
        sources/utils/device_local_image.cpp sources/utils/device_local_image.h
)

find_package(VulkanLoader REQUIRED)
target_link_libraries(my_renderer Vulkan::Loader)

find_package(glfw3 REQUIRED)
target_link_libraries(my_renderer glfw)

find_package(glm REQUIRED)
target_link_libraries(my_renderer glm::glm)

find_package(stb REQUIRED)
target_link_libraries(my_renderer stb::stb)

find_package(tinyobjloader REQUIRED)
target_link_libraries(my_renderer tinyobjloader::tinyobjloader)
