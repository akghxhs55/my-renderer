#include "window.h"


#include <stdexcept>


Window::Window(const char* windowTitle, const int width, const int height) :
    glfwWindow(createGlfwWindow(windowTitle, width, height))
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

GLFWwindow* Window::createGlfwWindow(const char* windowTitle, const int width, const int height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(width, height, windowTitle, nullptr, nullptr);
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
