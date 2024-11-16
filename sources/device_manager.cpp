#include "device_manager.h"


DeviceManager::DeviceManager(const PhysicalDeviceManager& physicalDeviceManager)
    : device(createDevice(physicalDeviceManager)),
      graphicsQueue(device.getQueue(physicalDeviceManager.graphicsQueueFamilyIndex, 0)),
      presentQueue(device.getQueue(physicalDeviceManager.presentQueueFamilyIndex, 0))
{
}

DeviceManager::~DeviceManager() = default;

vk::raii::Device DeviceManager::createDevice(const PhysicalDeviceManager& physicalDeviceManager)
{
    constexpr float queuePriority = 1.0f;

    const std::vector<uint32_t> queueFamilyIndices = physicalDeviceManager.getQueueFamilyIndices();
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
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
        .enabledExtensionCount = static_cast<uint32_t>(PhysicalDeviceManager::deviceExtensions.size()),
        .ppEnabledExtensionNames = PhysicalDeviceManager::deviceExtensions.data(),
        .pEnabledFeatures = &physicalDeviceFeatures
    };

    try
    {
        return physicalDeviceManager.physicalDevice.createDevice(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan device with error code: " + std::to_string(error.code().value()));
    }
}
