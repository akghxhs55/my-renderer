#ifndef MY_RENDERER_H
#define MY_RENDERER_H


#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include "environment.h"
#include "window.h"
#include "physical_device_manager.h"
#include "device_manager.h"
#include "swapchain_manager.h"
#include "render_pipeline.h"
#include "command_buffer_manager.h"


class MyRenderer {
public:
    MyRenderer();
    ~MyRenderer();
    void run();

private:
    static constexpr auto ApplicationName = "My Renderer";
    static constexpr uint32_t ApplicationVersion = vk::makeApiVersion(0, 0, 0, 0);
    static constexpr uint32_t MaxFramesInFlight = 2;

    const Environment environment;
    const Window window;
    const PhysicalDeviceManager physicalDeviceManager;
    const DeviceManager deviceManager;
    const SwapchainManager swapchainManager;
    const RenderPipeline renderPipeline;
    const CommandBufferManager commandBufferManager;
    const std::vector<vk::raii::Semaphore> imageAvailableSemaphore;
    const std::vector<vk::raii::Semaphore> renderFinishedSemaphore;
    const std::vector<vk::raii::Fence> inFlightFence;
    uint32_t currentFrame = 0;

    static std::vector<vk::raii::Semaphore> createSemaphores(const vk::raii::Device& device);
    static std::vector<vk::raii::Fence> createFences(const vk::raii::Device& device, const vk::FenceCreateFlags& flags);

    void drawFrame();
};


#endif //MY_RENDERER_H
