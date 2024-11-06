#ifndef MY_SHADER_H
#define MY_SHADER_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class MyShader {
public:
    MyShader();
    ~MyShader();
    void run();

private:
    using WindowPtr = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>;

    static constexpr auto ApplicationName = R"(My Shader)";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 1, 0, 0);

    static constexpr uint32_t Width = 800;
    static constexpr uint32_t Height = 600;

#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif

    static constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    WindowPtr window;
    vk::raii::Context context;
    vk::raii::Instance instance;
    std::optional<vk::raii::DebugUtilsMessengerEXT> debugMessenger;

    static WindowPtr initializeWindow();
    vk::raii::Instance initializeInstance() const;
    vk::raii::DebugUtilsMessengerEXT initializeDebugMessenger() const;
    void drawFrame();

    static std::vector<const char*> getRequiredExtensionNames();
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};


#endif //MY_SHADER_H
