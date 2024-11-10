#include "my_renderer.h"

#include <iostream>
#include <set>


MyRenderer::MyRenderer() :
    environment(ApplicationName, ApplicationVersion),
    window(environment.instance),
    physicalDeviceData(environment.instance, window.surface, deviceExtensions),
    device(createDevice(physicalDeviceData.physicalDevice, physicalDeviceData.graphicsQueueFamilyIndex.value())),
    presentQueue(device.getQueue(physicalDeviceData.presentQueueFamilyIndex.value(), 0)),
    swapchainData(window, physicalDeviceData, device)
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

vk::raii::Device MyRenderer::createDevice(const vk::raii::PhysicalDevice& physicalDevice, const uint32_t graphicsQueueFamilyIndex) const
{
    constexpr float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueCreateInfo{
        .queueFamilyIndex = graphicsQueueFamilyIndex,
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

void MyRenderer::drawFrame()
{
}
