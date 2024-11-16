#include "physical_device_manager.h"


#include <set>


const std::vector<const char*> PhysicalDeviceManager::deviceExtensions = {
    "VK_KHR_portability_subset",
    vk::KHRSwapchainExtensionName
};

PhysicalDeviceManager::PhysicalDeviceManager(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface) :
    physicalDevice(createPhysicalDevice(instance, surface)),
    graphicsQueueFamilyIndex(findQueueFamilies(physicalDevice, surface).graphics.value()),
    presentQueueFamilyIndex(findQueueFamilies(physicalDevice, surface).present.value())
{
}

PhysicalDeviceManager::~PhysicalDeviceManager() = default;

std::vector<uint32_t> PhysicalDeviceManager::getQueueFamilyIndices() const
{
    std::vector<uint32_t> queueFamilyIndices;
    queueFamilyIndices.push_back(graphicsQueueFamilyIndex);
    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
    {
        queueFamilyIndices.push_back(presentQueueFamilyIndex);
    }

    return queueFamilyIndices;
}

vk::SurfaceCapabilitiesKHR PhysicalDeviceManager::getSurfaceCapabilities(
    const vk::raii::SurfaceKHR& surface) const
{
    return physicalDevice.getSurfaceCapabilitiesKHR(surface);
}

std::vector<vk::SurfaceFormatKHR> PhysicalDeviceManager::getSurfaceFormats(const vk::raii::SurfaceKHR& surface) const
{
    return physicalDevice.getSurfaceFormatsKHR(surface);
}

std::vector<vk::PresentModeKHR> PhysicalDeviceManager::getSurfacePresentModes(const vk::raii::SurfaceKHR& surface) const
{
    return physicalDevice.getSurfacePresentModesKHR(surface);
}

vk::raii::PhysicalDevice PhysicalDeviceManager::createPhysicalDevice(const vk::raii::Instance& instance,
                                                                  const vk::raii::SurfaceKHR& surface)
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

bool PhysicalDeviceManager::isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice& physicalDevice,
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

PhysicalDeviceManager::QueueFamilyIndices PhysicalDeviceManager::findQueueFamilies(
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

bool PhysicalDeviceManager::checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice, const std::vector<const char*>& deviceExtensions)
{
    const std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const vk::ExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}