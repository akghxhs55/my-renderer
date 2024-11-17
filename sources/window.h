#ifndef WINDOW_H
#define WINDOW_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class Window {
public:
    static constexpr auto WindowTitle = "My Renderer";
    static constexpr uint32_t Width = 800;
    static constexpr uint32_t Height = 600;

    GLFWwindow* const glfwWindow;
    const vk::raii::SurfaceKHR surface;
private:
    bool framebufferResized = false;

public:
    explicit Window(const vk::raii::Instance& instance);
    ~Window();

    bool wasFramebufferResized() const;
    void resetFramebufferResized();

    bool shouldClose() const;

private:
    GLFWwindow* createGlfwWindow();
    vk::raii::SurfaceKHR createSurface(const vk::raii::Instance& instance) const;
};


#endif //WINDOW_H
