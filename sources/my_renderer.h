#ifndef MY_SHADER_H
#define MY_SHADER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "environment.h"
#include "window.h"
#include "swapchain_data.h"


class MyRenderer {
public:
    MyRenderer();
    ~MyRenderer();
    void run();

private:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const
        {
            return graphicsFamily.has_value();
        }
    };

    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);
    static constexpr std::array<const char*, 2> deviceExtensions = {
        "VK_KHR_portability_subset",
        vk::KHRSwapchainExtensionName
    };

    const Environment environment;
    const Window window;
    const vk::raii::PhysicalDevice physicalDevice;
    const std::vector<uint32_t> queueFamilyIndices;
    const vk::raii::Device device;
    const vk::raii::Queue presentQueue;
    const SwapchainData swapchainData;

    vk::raii::PhysicalDevice createPhysicalDevice() const;
    std::vector<uint32_t> createQueueFamilyIndices() const;
    vk::raii::Device createDevice() const;
    vk::raii::Queue createPresentQueue() const;

    void drawFrame();

    static bool isPhysicalDeviceSuitable(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface);
    static bool checkDeviceExtensionSupport(const vk::raii::PhysicalDevice& physicalDevice);
    static QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);
};


#endif //MY_SHADER_H
