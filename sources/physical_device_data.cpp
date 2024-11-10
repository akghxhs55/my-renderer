#include "physical_device_data.h"


#include <set>


PhysicalDeviceData::PhysicalDeviceData(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions) :
    physicalDevice(createPhysicalDevice(instance, surface, deviceExtensions)),
    graphicsQueueFamilyIndex(findQueueFamilies(physicalDevice, surface).graphics),
    presentQueueFamilyIndex(findQueueFamilies(physicalDevice, surface).present)
{
}

PhysicalDeviceData::~PhysicalDeviceData()
{
}

std::vector<uint32_t> PhysicalDeviceData::getQueueFamilyIndices() const
{
    std::vector<uint32_t> queueFamilyIndices;

    if (graphicsQueueFamilyIndex.has_value())
    {
        queueFamilyIndices.push_back(graphicsQueueFamilyIndex.value());
    }

    if (presentQueueFamilyIndex.has_value() and
        presentQueueFamilyIndex.value() != graphicsQueueFamilyIndex.value())
    {
        queueFamilyIndices.push_back(presentQueueFamilyIndex.value());
    }

    return queueFamilyIndices;
}

vk::raii::PhysicalDevice PhysicalDeviceData::createPhysicalDevice(const vk::raii::Instance& instance,
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

bool PhysicalDeviceData::isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice,
    const vk::raii::SurfaceKHR& surface, const std::vector<const char*>& deviceExtensions)
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    const bool extensionSupported = checkDeviceExtensionSupport(physicalDevice, deviceExtensions);

    bool swapChainAdequate = false;
    if (extensionSupported)
    {
        const std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface);
        const std::vector<vk::PresentModeKHR> presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

        swapChainAdequate = !formats.empty() and !presentModes.empty();
    }

    return indices.isComplete() and
           extensionSupported and
           swapChainAdequate;
}

PhysicalDeviceData::QueueFamilyIndices PhysicalDeviceData::findQueueFamilies(
    const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface)
{
    QueueFamilyIndices indices;

    const std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphics = i;
        }

        if (physicalDevice.getSurfaceSupportKHR(i, surface))
        {
            indices.present = i;
        }

        if (indices.isComplete())
        {
            break;
        }
    }

    return indices;
}

bool PhysicalDeviceData::checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice, const std::vector<const char*>& deviceExtensions)
{
    const std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const vk::ExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}