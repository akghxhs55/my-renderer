#include "device_context.h"


#include <set>


DeviceContext::DeviceContext(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions) :
    physicalDevice(selectPhysicalDevice(instance, surface, deviceExtensions)),
    graphicsQueueFamilyIndex(findGraphicsQueueFamilyIndex(physicalDevice).value()),
    presentQueueFamilyIndex(findPresentQueueFamilyIndex(physicalDevice, surface).value()),
    queueFamilyIndices(getUniqueValues({ graphicsQueueFamilyIndex, presentQueueFamilyIndex })),
    device(createDevice(physicalDevice, deviceExtensions)),
    graphicsQueue(device.getQueue(graphicsQueueFamilyIndex, 0)),
    presentQueue(device.getQueue(presentQueueFamilyIndex, 0)),
    surfaceCapabilities(physicalDevice.getSurfaceCapabilitiesKHR(*surface)),
    surfaceFormats(physicalDevice.getSurfaceFormatsKHR(*surface)),
    presentModes(physicalDevice.getSurfacePresentModesKHR(*surface))
{
}

DeviceContext::~DeviceContext() = default;

vk::raii::PhysicalDevice DeviceContext::selectPhysicalDevice(const vk::raii::Instance& instance,
    const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions)
{
    const std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

    for (const vk::raii::PhysicalDevice& physicalDevice : physicalDevices)
    {
        if (isPhysicalDeviceSuitable(physicalDevice, surface, deviceExtensions))
        {
            return physicalDevice;
        }
    }

    throw std::runtime_error("Failed to find a suitable GPU.");
}

vk::raii::Device DeviceContext::createDevice(const vk::raii::PhysicalDevice& physicalDevice, const std::vector<const char*>& deviceExtensions) const
{
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(queueFamilyIndices.size());

    constexpr float queuePriority = 1.0f;
    for (const auto& queueFamilyIndex : queueFamilyIndices)
    {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo{
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        });
    }

    constexpr vk::PhysicalDeviceFeatures physicalDeviceFeatures{};

    const vk::DeviceCreateInfo createInfo{
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &physicalDeviceFeatures
    };

    try
    {
        return vk::raii::Device(physicalDevice, createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan device with error code: " + std::to_string(error.code().value()));
    }
}

bool DeviceContext::isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice,
    const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions)
{
    const std::optional<uint32_t> graphicsQueueFamilyIndex = findGraphicsQueueFamilyIndex(physicalDevice);
    const std::optional<uint32_t> presentQueueFamilyIndex = findPresentQueueFamilyIndex(physicalDevice, surface);

    const bool extensionSupported = checkDeviceExtensionSupport(physicalDevice, deviceExtensions);

    bool swapChainAdequate = false;
    if (extensionSupported)
    {
        const std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(*surface);
        const std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

        swapChainAdequate = !formats.empty() and !presentModes.empty();
    }

    return graphicsQueueFamilyIndex.has_value() and
           presentQueueFamilyIndex.has_value() and
           extensionSupported and
           swapChainAdequate;
}

std::optional<uint32_t> DeviceContext::findGraphicsQueueFamilyIndex(const vk::raii::PhysicalDevice& physicalDevice)
{
    const std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            return i;
        }
    }

    return std::nullopt;
}

std::optional<uint32_t> DeviceContext::findPresentQueueFamilyIndex(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface)
{
    for (uint32_t i = 0; i < physicalDevice.getQueueFamilyProperties().size(); ++i)
    {
        if (physicalDevice.getSurfaceSupportKHR(i, surface))
        {
            return i;
        }
    }

    return std::nullopt;
}

bool DeviceContext::checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice, const std::vector<const char*>& deviceExtensions)
{
    const std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const vk::ExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

std::vector<uint32_t> DeviceContext::getUniqueValues(const std::vector<uint32_t>& values)
{
    std::set<uint32_t> uniqueValues(values.begin(), values.end());
    return std::vector<uint32_t>(uniqueValues.begin(), uniqueValues.end());
}
