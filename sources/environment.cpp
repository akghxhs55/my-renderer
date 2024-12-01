#include "environment.h"


#include <iostream>


Environment::Environment(const char* applicationName, const uint32_t applicationVersion) :
    context(),
    instance(createInstance(applicationName, applicationVersion)),
    debugMessenger(createDebugMessenger()),
    physicalDevice(selectPhysicalDevice()),
    queueFamilyIndices(findQueueFamilies(physicalDevice)),
    device(createDevice())
{
}

Environment::~Environment() = default;

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
        return context.createInstance(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan instance.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
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
        return instance.createDebugUtilsMessengerEXT(getDebugUtilsMessengerCreateInfo());
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan debug utils messenger.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
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
    std::cerr << "[Validation layer] " << pCallbackData->pMessage << std::endl;
    return vk::False;
}


vk::raii::PhysicalDevice Environment::selectPhysicalDevice() const
{
    const std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

    for (const vk::raii::PhysicalDevice& physicalDevice : physicalDevices)
    {
        if (isPhysicalDeviceSuitable(physicalDevice))
        {
            return physicalDevice;
        }
    }

    throw std::runtime_error("No suitable physical device found");
}

vk::raii::Device Environment::createDevice() const
{
    constexpr float queuePriority = 1.0f;

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for (const uint32_t queueFamilyIndex : queueFamilyIndices.getUniqueIndices())
    {
        queueCreateInfos.push_back({
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        });
    }

    constexpr vk::PhysicalDeviceFeatures physicalDeviceFeatures;

    const vk::DeviceCreateInfo createInfo{
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .pEnabledFeatures = &physicalDeviceFeatures
    };

    try
    {
        return physicalDevice.createDevice(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan logical device.\n Error code: " + std::to_string(error.code().value()) + "\n Error description: " + error.what());
    }
}

bool Environment::isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice)
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    return indices.isComplete();
}

Environment::QueueFamilyIndices Environment::findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice)
{
    QueueFamilyIndices indices;

    const std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }
    }

    return indices;
}
