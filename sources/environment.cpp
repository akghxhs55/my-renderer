#include "environment.h"


#include <iostream>


Environment::Environment(const char* applicationName, const uint32_t applicationVersion) :
    context(),
    instance(createInstance(applicationName, applicationVersion)),
    debugMessenger(createDebugMessenger())
{
}

Environment::~Environment()
{
}

vk::raii::Instance Environment::createInstance(const char* applicationName, const uint32_t applicationVersion) const
{
    void* pNext = nullptr;
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = getDebugUtilsMessengerCreateInfo();
    if constexpr (enabledDebug)
    {
        pNext = &debugUtilsMessengerCreateInfo;
    }

    vk::ApplicationInfo applicationInfo{
        .pApplicationName = applicationName,
        .applicationVersion = applicationVersion,
        .pEngineName = EngineName,
        .engineVersion = EngineVersion,
        .apiVersion = vk::ApiVersion13
    };

    std::vector<const char*> instanceLayers;
    if (enabledDebug)
    {
        for (const char* layer : validationLayers)
        {
            instanceLayers.push_back(layer);
        }
    }

    std::vector<const char*> instanceExtensions = getRequiredExtensionNames();

    const vk::InstanceCreateInfo createInfo{
        .pNext = pNext,
        .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data()
    };

    try
    {
        return vk::raii::Instance(context, createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan instance with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::DebugUtilsMessengerEXT Environment::createDebugMessenger() const
{
    if constexpr (!enabledDebug)
    {
        return nullptr;
    }

    try
    {
        return vk::raii::DebugUtilsMessengerEXT(instance, getDebugUtilsMessengerCreateInfo());
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan debug utils messenger with error code: " + std::to_string(error.code().value()));
    }
}

std::vector<const char*> Environment::getRequiredExtensionNames()
{
    std::vector<const char*> extensionNames;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        extensionNames.emplace_back(glfwExtensions[i]);
    }

    if constexpr (enabledDebug)
    {
        extensionNames.emplace_back(vk::EXTDebugUtilsExtensionName);
    }

    extensionNames.emplace_back(vk::KHRPortabilityEnumerationExtensionName);

    return extensionNames;
}

vk::DebugUtilsMessengerCreateInfoEXT Environment::getDebugUtilsMessengerCreateInfo()
{
    return vk::DebugUtilsMessengerCreateInfoEXT{
        .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        .pfnUserCallback = debugCallback,
        .pUserData = nullptr
    };
}

vk::Bool32 Environment::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}
