#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class Environment {
private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;

        bool isComplete() const
        {
            return graphicsFamily.has_value();
        }

        std::vector<uint32_t> getUniqueIndices() const
        {
            std::vector<uint32_t> indices;
            if (graphicsFamily.has_value())
            {
                indices.push_back(graphicsFamily.value());
            }
            return indices;
        }
    };

private:
    const vk::raii::Context context;
public:
    const vk::raii::Instance instance;
private:
    const std::optional<vk::raii::DebugUtilsMessengerEXT> debugMessenger;
public:
    const vk::raii::PhysicalDevice physicalDevice;
private:
    const QueueFamilyIndices queueFamilyIndices;
public:
    const vk::raii::Device device;
    const vk::raii::Queue graphicsQueue;

public:
    Environment(const char* applicationName, const uint32_t applicationVersion);
    ~Environment();

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

    static std::vector<const char*> getRequiredExtensionNames();
    static vk::DebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfo();
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    static bool isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice);
    static QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice);

};


#endif //ENVIRONMENT_H
