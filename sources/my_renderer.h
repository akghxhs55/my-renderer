#ifndef MY_RENDERER_H
#define MY_RENDERER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "environment.h"
#include "window.h"
#include "physical_device_manager.h"
#include "device_manager.h"
#include "swapchain_data.h"
#include "render_pipeline.h"


class MyRenderer {
public:
    MyRenderer();
    ~MyRenderer();
    void run() const;

private:
    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);
    const std::vector<const char*> deviceExtensions = {
        "VK_KHR_portability_subset",
        vk::KHRSwapchainExtensionName
    };

    const Environment environment;
    const Window window;
    const PhysicalDeviceManager physicalDeviceManager;
    const DeviceManager deviceManager;
    const SwapchainData swapchainData;
    const RenderPipeline renderPipeline;
    const vk::raii::CommandPool commandPool;
    const vk::raii::CommandBuffer commandBuffer;
    const vk::raii::Semaphore imageAvailableSemaphore;
    const vk::raii::Semaphore renderFinishedSemaphore;
    const vk::raii::Fence inFlightFence;

    static vk::raii::CommandPool createCommandPool(const vk::raii::Device& device, const uint32_t& queueFamilyIndex);
    static vk::raii::CommandBuffer createCommandBuffer(const vk::raii::Device& device, const vk::raii::CommandPool& commandPool);
    static vk::raii::Semaphore createSemaphore(const vk::raii::Device& device);
    static vk::raii::Fence createFence(const vk::raii::Device& device, const vk::FenceCreateFlags& flags);

    void drawFrame() const;

    static void recordCommandBuffer(const vk::raii::CommandBuffer& commandBuffer, const uint32_t& imageIndex, const RenderPipeline& renderPipeline, const vk::Extent2D& swapchainExtent);
};


#endif //MY_RENDERER_H
