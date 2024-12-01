#ifndef WINDOW_H
#define WINDOW_H


#define GLFW_INCLUDE_VULKAN
#include <utility>
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class Window {
public:
    GLFWwindow* const glfwWindow;

    Window(const char* windowTitle, const int width, const int height);
    ~Window();

    std::pair<int, int> getFramebufferSize() const;
    bool shouldClose() const;
    bool wasFramebufferResized() const;
    void resetFramebufferResized();

    vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance) const;

private:
    bool framebufferResized = false;

    GLFWwindow* createGlfwWindow(const char* windowTitle, const int width, const int height);
};


#endif //WINDOW_H
