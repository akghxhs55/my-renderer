#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "window.h"


class Environment {
private:
    struct QueueFamilyIndices {
        const std::optional<uint32_t> graphicsFamily;
        const std::optional<uint32_t> presentFamily;

        bool isComplete() const
        {
            return graphicsFamily.has_value() and
                    presentFamily.has_value();
        }

        std::vector<uint32_t> getUniqueIndices() const
        {
            std::vector<uint32_t> indices;
            if (graphicsFamily.has_value())
            {
                indices.push_back(graphicsFamily.value());
            }
            if (presentFamily.has_value() and presentFamily.value() != graphicsFamily.value())
            {
                indices.push_back(presentFamily.value());
            }
            return indices;
        }
    };
    struct SwapchainDetails
    {
        const vk::SurfaceCapabilitiesKHR capabilities;
        const std::vector<vk::SurfaceFormatKHR> formats;
        const std::vector<vk::PresentModeKHR> presentModes;

        bool isComplete() const
        {
            return !formats.empty() and
                    !presentModes.empty();
        }
    };

private:
    const Window& window;
    const vk::raii::Context context;
    const vk::raii::Instance instance;
    const std::optional<vk::raii::DebugUtilsMessengerEXT> debugMessenger;
    const vk::raii::SurfaceKHR surface;
    const vk::raii::PhysicalDevice physicalDevice;
    const QueueFamilyIndices queueFamilyIndices;
public:
    const vk::PhysicalDeviceProperties physicalDeviceProperties;
    const vk::raii::Device device;
    const vk::raii::Queue graphicsQueue;
    const vk::raii::Queue presentQueue;
private:
    const vk::raii::CommandPool graphicsCommandPool;
public:
    const vk::SurfaceFormatKHR swapchainSurfaceFormat;
private:
    vk::Extent2D swapchainExtent;
    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::raii::ImageView> swapchainImageViews;

public:
    Environment(const Window& window, const char* applicationName, const uint32_t applicationVersion, const uint32_t maxFramesInFlight);
    ~Environment();

    std::vector<vk::raii::Framebuffer> createSwapchainFramebuffers(const vk::raii::RenderPass& renderPass) const;
    std::vector<vk::raii::CommandBuffer> createGraphicsCommandBuffers(const uint32_t count, const vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) const;
    vk::raii::Semaphore createSemaphore(const vk::SemaphoreCreateFlags flags = {}) const;
    vk::raii::Fence createFence(const vk::FenceCreateFlags flags = {}) const;

    vk::Viewport getViewport() const;
    vk::Rect2D getScissor() const;
    vk::Extent2D getSwapchainExtent() const;
    const vk::raii::SwapchainKHR& getSwapchain() const;

    void recreateSwapchain();
    uint32_t findMemoryType(const uint32_t typeFilter, const vk::MemoryPropertyFlags properties) const;
    vk::raii::CommandBuffer beginSingleTimeCommands() const;
    void submitSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;

private:
    static constexpr auto EngineName = "No Engine";
    static constexpr uint32_t EngineVersion = vk::makeApiVersion(0, 0, 0, 0);
#ifdef NDEBUG
    static constexpr bool enabledDebug = false;
#else
    static constexpr bool enabledDebug = true;
#endif
    static constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    static constexpr std::array<const char*, 2> deviceExtensions = {
        "VK_KHR_portability_subset",
        vk::KHRSwapchainExtensionName
    };

    vk::raii::Instance createInstance(const char* applicationName, const uint32_t applicationVersion) const;
    vk::raii::DebugUtilsMessengerEXT createDebugMessenger() const;
    vk::raii::PhysicalDevice selectPhysicalDevice() const;
    vk::raii::Device createDevice() const;
    vk::raii::CommandPool createCommandPool(const uint32_t queueFamilyIndex) const;
    vk::raii::SwapchainKHR createSwapchain() const;
    std::vector<vk::raii::ImageView> createSwapchainImageViews() const;

    static std::vector<const char*> getRequiredExtensionNames();
    static vk::DebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfo();
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    bool isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice) const;
    static bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice);
    QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice) const;
    SwapchainDetails querySwapchainSupport(const vk::raii::PhysicalDevice& physicalDevice) const;
    static vk::SurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;
};


#endif //ENVIRONMENT_H
