#include "window.h"


#include <stdexcept>


Window::Window(const char* windowTitle, const int width, const int height) :
    glfwWindow(createGlfwWindow(windowTitle, width, height)),
    framebufferResized(false)
{
}

Window::~Window()
{
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

std::pair<int, int> Window::getFramebufferSize() const
{
    int width, height;
    glfwGetFramebufferSize(glfwWindow, &width, &height);

    return { width, height };
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(glfwWindow);
}

bool Window::wasFramebufferResized() const
{
    return framebufferResized;
}

void Window::resetFramebufferResized()
{
    framebufferResized = false;
}

vk::raii::SurfaceKHR Window::createSurface(const vk::raii::Instance& instance) const
{
    VkSurfaceKHR surface;
    if (const VkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(*instance), glfwWindow, nullptr, &surface); result == VK_SUCCESS)
    {
        return vk::raii::SurfaceKHR(instance, surface);
    }
    else
    {
        throw std::runtime_error("Failed to create window surface.\n Error code: " + std::to_string(result));
    }
}

GLFWwindow* Window::createGlfwWindow(const char* windowTitle, const int width, const int height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("Failed to create GLFW window.");
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* _window, int, int)
    {
        const auto windowPtr = static_cast<Window*>(glfwGetWindowUserPointer(_window));
        windowPtr->framebufferResized = true;
    });

    return window;
}
