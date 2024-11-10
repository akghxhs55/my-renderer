#ifndef MY_SHADER_H
#define MY_SHADER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "environment.h"
#include "window.h"
#include "physical_device_data.h"
#include "swapchain_data.h"


class MyRenderer {
public:
    MyRenderer();
    ~MyRenderer();
    void run();

private:
    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);
    const std::vector<const char*> deviceExtensions = {
        "VK_KHR_portability_subset",
        vk::KHRSwapchainExtensionName
    };

    const Environment environment;
    const Window window;
    const PhysicalDeviceData physicalDeviceData;
    const vk::raii::Device device;
    const vk::raii::Queue presentQueue;
    const SwapchainData swapchainData;

    vk::raii::Device createDevice(const vk::raii::PhysicalDevice& physicalDevice, const uint32_t graphicsQueueFamilyIndex) const;

    void drawFrame();
};


#endif //MY_SHADER_H
