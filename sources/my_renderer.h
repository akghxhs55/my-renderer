#ifndef MY_SHADER_H
#define MY_SHADER_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class MyRenderer {
public:
    MyRenderer();
    ~MyRenderer();
    void run();

private:
    using WindowPtr = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>;
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const
        {
            return graphicsFamily.has_value();
        }
    };
    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    static constexpr auto ApplicationName = R"(My Shader)";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);
    static constexpr uint32_t Width = 800;
    static constexpr uint32_t Height = 600;

    static constexpr auto EngineName = R"(No Engine)";
    static constexpr uint32_t EngineVersion = vk::makeApiVersion(0, 0, 0, 0);
#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif
    static constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    static constexpr std::array<const char*, 2> deviceExtensions = {
        "VK_KHR_portability_subset",
        vk::KHRSwapchainExtensionName
    };

    WindowPtr window;
    vk::raii::Context context;
    vk::raii::Instance instance;
    std::optional<vk::raii::DebugUtilsMessengerEXT> debugMessenger;
    vk::raii::SurfaceKHR surface;
    vk::raii::PhysicalDevice physicalDevice;
    vk::raii::Device device;
    vk::raii::Queue presentQueue;
    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> swapchainImages;
    vk::Format swapchainImageFormat;
    vk::Extent2D swapchainExtent;
    std::vector<vk::raii::ImageView> swapchainImageViews;

    static WindowPtr initializeWindow();
    vk::raii::Instance initializeInstance() const;
    vk::raii::DebugUtilsMessengerEXT initializeDebugMessenger() const;
    vk::raii::SurfaceKHR initializeSurface() const;
    vk::raii::PhysicalDevice initializePhysicalDevice() const;
    vk::raii::Device initializeDevice() const;
    vk::raii::Queue initializePresentQueue() const;
    vk::raii::SwapchainKHR initializeSwapchain() const;
    std::vector<vk::Image> initializeSwapchainImages() const;
    vk::Format initializeSwapchainImageFormat() const;
    vk::Extent2D initializeSwapchainExtent() const;
    std::vector<vk::raii::ImageView> initializeSwapchainImageViews() const;

    void drawFrame();

    static std::vector<const char*> getRequiredExtensionNames();
    static vk::DebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfo();
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    static bool isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface);
    static bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice);
    static QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);
    static SwapChainSupportDetails querySwapChainSupport(const vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface);
    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities,  GLFWwindow* window);
};


#endif //MY_SHADER_H
