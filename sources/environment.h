#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>


class Environment {
private:
    const vk::raii::Context context;
public:
    const vk::raii::Instance instance;
private:
    const std::optional<vk::raii::DebugUtilsMessengerEXT> debugMessenger;

public:
    Environment(const char* applicationName, const uint32_t applicationVersion);
    ~Environment();

private:
    static constexpr auto EngineName = R"(No Engine)";
    static constexpr uint32_t EngineVersion = vk::makeApiVersion(0, 0, 0, 0);
#ifdef NDEBUG
    static constexpr bool enabledDebug = false;
#else
    static constexpr bool enabledDebug = true;
#endif
    static constexpr std::array<const char*, 1> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    vk::raii::Instance createInstance(const char* applicationName, const uint32_t applicationVersion) const;
    vk::raii::DebugUtilsMessengerEXT createDebugMessenger() const;

    static std::vector<const char*> getRequiredExtensionNames();
    static vk::DebugUtilsMessengerCreateInfoEXT getDebugUtilsMessengerCreateInfo();
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};


#endif //ENVIRONMENT_H
