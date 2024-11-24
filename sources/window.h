#ifndef WINDOW_H
#define WINDOW_H


#define GLFW_INCLUDE_VULKAN
#include <utility>
#include <GLFW/glfw3.h>


class Window {
public:
    GLFWwindow* const glfwWindow;

public:
    Window(const char* windowTitle, const int width, const int height);
    ~Window();

    std::pair<int, int> getFramebufferSize() const;

    bool wasFramebufferResized() const;
    void resetFramebufferResized();

    bool shouldClose() const;

private:
    bool framebufferResized = false;

    GLFWwindow* createGlfwWindow(const char* windowTitle, const int width, const int height);
};


#endif //WINDOW_H
