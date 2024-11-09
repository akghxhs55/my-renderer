#include "my_renderer.h"

#include <iostream>
#include <set>


MyRenderer::MyRenderer() :
    environment(ApplicationName, ApplicationVersion),
    window(environment.instance),
    physicalDevice(createPhysicalDevice()),
    queueFamilyIndices(createQueueFamilyIndices()),
    device(createDevice()),
    presentQueue(createPresentQueue()),
    swapchainData(window.glfwWindow, window.surface, physicalDevice, queueFamilyIndices, device)
{
}

MyRenderer::~MyRenderer()
{
}

void MyRenderer::run()
{
    while (!glfwWindowShouldClose(window.glfwWindow))
    {
        glfwPollEvents();
        drawFrame();
    }
}

vk::raii::PhysicalDevice MyRenderer::createPhysicalDevice() const
{
    const std::vector<vk::raii::PhysicalDevice> physicalDevices = environment.instance.enumeratePhysicalDevices();

    for (const vk::raii::PhysicalDevice& physicalDevice : physicalDevices)
    {
        if (isPhysicalDeviceSuitable(physicalDevice, window.surface))
        {
            return physicalDevice;
        }
    }

    throw std::runtime_error("Failed to find a suitable GPU.");
}

std::vector<uint32_t> MyRenderer::createQueueFamilyIndices() const
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, window.surface);

    std::vector<uint32_t> queueFamilyIndices;
    if (indices.graphicsFamily.has_value())
    {
        queueFamilyIndices.push_back(indices.graphicsFamily.value());
    }

    if (indices.presentFamily.has_value() and
        indices.presentFamily.value() != indices.graphicsFamily.value())
    {
        queueFamilyIndices.push_back(indices.presentFamily.value());
    }

    return queueFamilyIndices;
}

vk::raii::Device MyRenderer::createDevice() const
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, window.surface);
    constexpr float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueCreateInfo{
        .queueFamilyIndex = indices.graphicsFamily.value(),
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::PhysicalDeviceFeatures physicalDeviceFeatures{};

    const vk::DeviceCreateInfo createInfo{
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &physicalDeviceFeatures
    };

    try
    {
        return physicalDevice.createDevice(createInfo);
    }
    catch (const vk::SystemError& error)
    {
        throw std::runtime_error("Failed to create Vulkan device with error code: " + std::to_string(error.code().value()));
    }
}

vk::raii::Queue MyRenderer::createPresentQueue() const
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, window.surface);

    return device.getQueue(indices.presentFamily.value(), 0);
}

void MyRenderer::drawFrame()
{
}

bool MyRenderer::isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface)
{
    const QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    const bool extensionSupported = checkDeviceExtensionSupport(physicalDevice);

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

bool MyRenderer::checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice)
{
    const std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const vk::ExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

MyRenderer::QueueFamilyIndices MyRenderer::findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface)
{
    QueueFamilyIndices indices;

    const std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }

        if (physicalDevice.getSurfaceSupportKHR(i, surface))
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }
    }

    return indices;
}
