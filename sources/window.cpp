#include "window.h"


#include <stdexcept>


Window::Window(const vk::raii::Instance& instance) :
    glfwWindow(createGlfwWindow()),
    surface(createSurface(instance))
{
}

Window::~Window()
{
    glfwDestroyWindow(glfwWindow);
}

bool Window::wasFramebufferResized() const
{
    return framebufferResized;
}

void Window::resetFramebufferResized()
{
    framebufferResized = false;
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(glfwWindow);
}

GLFWwindow* Window::createGlfwWindow()
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(Width, Height, WindowTitle, nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("Failed to create GLFW window.");
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* _window, int width, int height)
    {
        const auto windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(_window));
        windowPtr->framebufferResized = true;
    });

    return window;
}

vk::raii::SurfaceKHR Window::createSurface(const vk::raii::Instance& instance) const
{
    VkSurfaceKHR surface;
    if (const VkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(*instance), glfwWindow, nullptr, &surface);
        result == VK_SUCCESS)
    {
        return vk::raii::SurfaceKHR(instance, surface);
    }
    else
    {
        throw std::runtime_error("Failed to create window surface with error code: " + std::to_string(result));
    }
}
